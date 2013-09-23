/* Horizon bytecode compiler
 * Copyright (C) 2010 James Molloy
 *
 * Horizon is open source software, released under the terms of the Non-Profit
 * Open Software License 3.0. You should have received a copy of the
 * licensing information along with the source code distribution. If you
 * have not received a copy of the license, please refer to the Horizon
 * project website.
 *
 * Please note that if you modify this file, the license requires you to
 * ADD your name to the list of contributors. This boilerplate is not the
 * license itself; please refer to the copy of the license you have received
 * for complete terms.
 */

#include <iostream>
#include <horizon/Baremetal/kiwi.h>
#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/PhysicalMemoryManager.h>
#include <horizon/Baremetal/InterruptManager.h>
#include <horizon/Baremetal/syscallHandlers.h>
#include <horizon/Baremetal/SpecialFile.h>

#include <horizon/Function.h>
#include <horizon/Module.h>
#include <horizon/Package.h>
#include <horizon/Bytecode/Bytecoder.h>
#include <horizon/verify.h>
#include <horizon/Codegen/FunctionInstantiation.h>
#include <horizon/Codegen/CompileManager.h>
#include <horizon/Codegen/Array.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Target/TargetSelect.h>

#include <ios>
#include <fstream>
#include <string>

#include <boost/foreach.hpp>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>

using namespace llvm;
using namespace horizon;

static cl::opt<bool>
DisableOutput("disable-output", cl::desc("Disable output"), cl::init(false));

static cl::opt<bool>
Optimise("O", cl::desc("Optimize LLVM assembly"), cl::init(false));

static cl::opt<bool>
Verbose("v", cl::desc("Verbose mode"), cl::init(false));

static cl::opt<bool>
DisableVerify("disable-verify", cl::Hidden,
              cl::desc("Do not run verifier on input HBC (dangerous!)"));

static cl::opt<bool>
Time("time", cl::desc("Time _main() function execution."), cl::init(false));

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input .hbc file>"), cl::init("-"));

static cl::list<std::string>
Argv(cl::ConsumeAfter, cl::desc("<program arguments>..."));

ErrorManager errors;

extern "C" void CallStart();

extern void InitialisePthreads();
extern void InitialiseRuntimeAddresses();

extern "C" void kmain(kernel_args_t *args, unsigned int cpu) {
	if(cpu != args->boot_cpu) {
		while(1);
	}

	DebugStream::Initialise();
	dout << "horizon: *** kernel initialising ***\n";


	PhysicalMemoryManager::instance().Initialise(args);
	InterruptManager::instance().Initialise();

	InitialiseFPU();

	InitialiseSpecialFiles(args);
	InitialiseSyscallHandlers();
	InitialisePthreads();

	CallStart();

	for(;;) ;
}

int main() {
	/* Here we can assume the entire operating environment is up and running, including C and C++ standard libraries. */

	InitialiseRuntimeAddresses();

	static const char *argv[] = {"horizon", "dummyInputFile.hbc"};
	static const int argc = 1;

	cl::ParseCommandLineOptions(argc, (char**)argv, "horizon .hbc interpreter for unhosted environments.\n");

	FILE *stream  = fopen("/module", "rb");
	if(!stream) {
		abort();
	}
	fseek(stream, 0, SEEK_END);
	size_t sz = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	char *c = new char[sz];
	fread(c, 1, sz, stream);
	fclose(stream);

	Bytecoder bc(std::cout, c, sz, &errors);

	horizon::Module *m = bc.ReadModule();

	if(!m || errors.AreErrors()) {
		errors.Dump(std::cerr);
		return 1;
	}
	
	if(!DisableVerify) {
		bool b = VerifyModule(errors, *m);
		if(!b || errors.AreErrors()) {
			errors.Dump(std::cerr);
			return 1;
		}
	}

	ErrorManager errors;

	llvm::LLVMContext &context = llvm::getGlobalContext();
	llvm::Module *llvm_module = new llvm::Module("test", context);
	CompileManager manager(*m, llvm_module, Verbose, Optimise);

	if(!manager.RegisterMemberFunctions(errors)) {
		errors.Dump(std::cerr);
		return 1;
	}
	
	InitializeNativeTarget();

	std::string error_str;
	llvm::EngineBuilder eb(llvm_module);
	eb.setErrorStr(&error_str);
	eb.setEngineKind(EngineKind::JIT);
	llvm::ExecutionEngine *engine = eb.create();
	Assert(engine);

	/* Iterate through the root package, looking for functions. */
	const Package::SymbolMap &symmap = m->RootPackage()->AllSymbols();
	for(Package::SymbolMap::const_iterator it = symmap.begin();
	    it != symmap.end();
	    it++) {
		if(it->first != "_main") {
			continue;
		}

		const Symbol *sym = it->second;
		const horizon::Function *fn = sym->Get<horizon::Function>();
		if(!fn) {
			continue;
		}
		
		/* Function found, tally ho! */
		horizon::Type::ConcreteTypeMap ct;
		FunctionSignature *sig = fn->GetSignatures().front();
		FunctionInstantiation instance(*m, it->first, sig);
		if(!instance.InferTypes(errors, manager, sig->ParameterType(), sig->ReturnType(), ct)) {
			std::cerr << "Failed type inference" << std::endl;
			errors.Dump(std::cerr);
			return 1;
		}

		instance.RunAuxiliaryPasses(errors, manager);

		llvm::Value *compiled_fn = instance.Codegen(errors, manager);
		if(!compiled_fn) {
			std::cerr << "Failed code generation" << std::endl;
			errors.Dump(std::cerr);
			return 1;
		}

		llvm::Function *compiled_fn_ = llvm::dyn_cast<llvm::Function>(compiled_fn);
		void *fnptr = engine->getPointerToFunction(compiled_fn_);
		
		typedef int (*MainType)(uint64_t _this, uint64_t argc, horizon::Array<uint8_t> *argv);
		MainType fnmain = reinterpret_cast<MainType>(fnptr);

		/* Construct array. */
		horizon::Array<uint8_t> a;
		a.bounds = Argv.size();
		a.array = new uint8_t[a.bounds];
		int i = 0;
		BOOST_FOREACH(const std::string str, Argv) {
			a.array[i++] = atoi(str.c_str());
		}


		struct tms tms;
		time_t starttime=0, stoptime=0;
		if(Time) {
			times(&tms);
			starttime = tms.tms_utime;
		}

		int ret = fnmain(0, a.bounds, &a);

		if(Time) {
			times(&tms);
			stoptime = tms.tms_utime;
			time_t t = stoptime - starttime;
			
			int hz = sysconf(_SC_CLK_TCK);

			printf("Time(ms): %ld\n", t*1000/hz);
		}

		/** \bug Should	destroy	all our	contexts, but cannot due to a
		 *  	 bug in	LLVM, r103769 */
		//llvm::llvm_shutdown();

		printf("horizon: *** Exiting with code %d\n", ret);

		return ret;
	}


	return 0;
}

