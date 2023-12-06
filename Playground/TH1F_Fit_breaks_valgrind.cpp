// g++ TH1F_Fit_breaks_valgrind.cpp $(root-config --glibs --cflags --libs) -o test -g -DDebug -Wall -Wextra -std=c++14

#include "TH1F.h"
#include "TF1.h"

int main()
{
  std::cout << "Spectra" << std::endl;
  TH1F* spectra = new TH1F("test","test",100,-1,1);
  spectra -> FillRandom("gaus",100000);
  double mean = 0;

  float pospic = 0; 
  float amppic = 1200; 
  float dump_sigma = 0.2;

  std::cout << "Function" << std::endl;
  TF1 *gaus_pol0 = new TF1("gaus+pol0","gaus(0)+pol0(3)",-1,1);
  gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
  gaus_pol0 -> SetRange(-1,1);

  // The error comes here !! See below the output of valgrind
  std::cout << "Fit" << std::endl;
  spectra -> Fit(gaus_pol0,"R+q");

  std::cout << "Extract parameters" << std::endl;
  mean = gaus_pol0->GetParameter(1);
  std::cout << mean << std::endl;

  std::cout << "Delete pointers" << std::endl;
  delete spectra;
  delete gaus_pol0;

  std::cout << "Finish" << std::endl;

  return -1;    
}

// valgrind: m_debuginfo/debuginfo.c:914 (truncate_DebugInfoMapping_overlaps): Assertion '!overlap' failed.

// host stacktrace:

// ==2957==    at 0x58047B6A: show_sched_status_wrk (m_libcassert.c:406)

// ==2957==    by 0x58047C87: report_and_quit (m_libcassert.c:477)

// ==2957==    by 0x58047E17: vgPlain_assert_fail (m_libcassert.c:543)

// ==2957==    by 0x5807A82D: truncate_DebugInfoMapping_overlaps (debuginfo.c:914)

// ==2957==    by 0x5807A82D: di_notify_ACHIEVE_ACCEPT_STATE (debuginfo.c:965)

// ==2957==    by 0x5807A82D: vgPlain_di_notify_mmap (debuginfo.c:1319)

// ==2957==    by 0x580AD6E8: vgModuleLocal_generic_PRE_sys_mmap (syswrap-generic.c:2400)

// ==2957==    by 0x580B932F: vgSysWrap_amd64_linux_sys_mmap_before (syswrap-amd64-linux.c:413)

// ==2957==    by 0x580A9041: vgPlain_client_syscall (syswrap-main.c:1914)

// ==2957==    by 0x580A553A: handle_syscall (scheduler.c:1208)

// ==2957==    by 0x580A6F82: vgPlain_scheduler (scheduler.c:1526)

// ==2957==    by 0x580F6640: thread_wrapper (syswrap-linux.c:101)

// ==2957==    by 0x580F6640: run_a_thread_NORETURN (syswrap-linux.c:154)



// sched status:

//   running_tid=1



// Thread 1: status = VgTs_Runnable syscall 9 (lwpid 2957)

// ==2957==    at 0x401BE82: __mmap64 (mmap64.c:59)

// ==2957==    by 0x401BE82: mmap (mmap64.c:47)

// ==2957==    by 0x400758E: _dl_map_segments (dl-map-segments.h:94)

// ==2957==    by 0x400758E: _dl_map_object_from_fd (dl-load.c:1186)

// ==2957==    by 0x4009274: _dl_map_object (dl-load.c:2236)

// ==2957==    by 0x4013D41: dl_open_worker (dl-open.c:513)

// ==2957==    by 0x5CD7A8F: _dl_catch_exception (dl-error-skeleton.c:208)

// ==2957==    by 0x40138F9: _dl_open (dl-open.c:837)

// ==2957==    by 0x5EDA257: dlopen_doit (dlopen.c:66)

// ==2957==    by 0x5CD7A8F: _dl_catch_exception (dl-error-skeleton.c:208)

// ==2957==    by 0x5CD7B4E: _dl_catch_error (dl-error-skeleton.c:227)

// ==2957==    by 0x5EDAA64: _dlerror_run (dlerror.c:170)

// ==2957==    by 0x5EDA2E3: dlopen@@GLIBC_2.2.5 (dlopen.c:87)

// ==2957==    by 0x7388454: cling::utils::platform::DLOpen(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >*) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x728D312: cling::DynamicLibraryManager::loadLibrary(llvm::StringRef, bool, bool) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x71D17B7: TCling::Load(char const*, bool) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x4C7FEEA: TSystem::Load(char const*, char const*, bool) (in /home/corentin/Installation/root/root_install/lib/libCore.so)

// ==2957==    by 0x71E5F48: TCling::LazyFunctionCreatorAutoload(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x7319F35: cling::IncrementalExecutor::NotifyLazyFunctionCreators(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) const (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x73211F7: llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> > llvm::orc::lookupWithLegacyFn<cling::IncrementalJIT::IncrementalJIT(cling::IncrementalExecutor&, std::unique_ptr<llvm::TargetMachine, std::default_delete<llvm::TargetMachine> >, std::function<void (unsigned long, std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> >)>)::{lambda(std::shared_ptr<llvm::orc::AsynchronousSymbolQuery>, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >)#2}::operator()(std::shared_ptr<llvm::orc::AsynchronousSymbolQuery>, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >) const::{lambda(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)#1}>(llvm::orc::ExecutionSession&, llvm::orc::AsynchronousSymbolQuery&, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> > const&, cling::IncrementalJIT::IncrementalJIT(cling::IncrementalExecutor&, std::unique_ptr<llvm::TargetMachine, std::default_delete<llvm::TargetMachine> >, std::function<void (unsigned long, std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> >)>)::{lambda(std::shared_ptr<llvm::orc::AsynchronousSymbolQuery>, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >)#2}::operator()(std::shared_ptr<llvm::orc::AsynchronousSymbolQuery>, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >) const::{lambda(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)#1}) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x732211A: llvm::orc::LambdaSymbolResolver<cling::IncrementalJIT::IncrementalJIT(cling::IncrementalExecutor&, std::unique_ptr<llvm::TargetMachine, std::default_delete<llvm::TargetMachine> >, std::function<void (unsigned long, std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> >)>)::{lambda(llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> > const&)#1}, cling::IncrementalJIT::DenseSet(cling::IncrementalExecutor&, std::unique_ptr<llvm::TargetMachine, std::default_delete<llvm::TargetMachine> >, std::function<void (unsigned long, std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> >)>)::{lambda(std::shared_ptr<llvm::orc::AsynchronousSymbolQuery>, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >)#2}>::lookup(llvm::orc::AsynchronousSymbolQuery, llvm::DenseSet<llvm::orc::SymbolStringPtr, llvm::DenseMapInfo<llvm::orc::SymbolStringPtr> >) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x839CEC6: llvm::orc::JITSymbolResolverAdapter::lookup(std::set<llvm::StringRef, std::less<llvm::StringRef>, std::allocator<llvm::StringRef> > const&, std::function<void (llvm::Expected<std::map<llvm::StringRef, llvm::JITEvaluatedSymbol, std::less<llvm::StringRef>, std::allocator<std::pair<llvm::StringRef const, llvm::JITEvaluatedSymbol> > > >)>) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x83B4794: llvm::RuntimeDyldImpl::resolveExternalSymbols() (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x83B522F: llvm::RuntimeDyldImpl::resolveRelocations() (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x83B559C: llvm::RuntimeDyld::finalizeWithMemoryManagerLocking() (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x731D0A7: llvm::orc::LegacyRTDyldObjectLinkingLayer::ConcreteLinkedObject<std::shared_ptr<llvm::RuntimeDyld::MemoryManager> >::finalize() (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x731C151: std::_Function_handler<llvm::Expected<unsigned long> (), llvm::orc::LegacyRTDyldObjectLinkingLayer::ConcreteLinkedObject<std::shared_ptr<llvm::RuntimeDyld::MemoryManager> >::getSymbolMaterializer(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)::{lambda()#1}>::_M_invoke(std::_Any_data const&) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x7326CBA: std::_Function_handler<llvm::Expected<unsigned long> (), llvm::orc::LazyEmittingLayer<llvm::orc::LegacyIRCompileLayer<cling::IncrementalJIT::RemovableObjectLinkingLayer, llvm::orc::SimpleCompiler> >::EmissionDeferredModule::find(llvm::StringRef, bool, llvm::orc::LegacyIRCompileLayer<cling::IncrementalJIT::RemovableObjectLinkingLayer, llvm::orc::SimpleCompiler>&)::{lambda()#1}>::_M_invoke(std::_Any_data const&) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x731870C: cling::IncrementalExecutor::executeWrapper(llvm::StringRef, cling::Value*) const (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x72A29F0: cling::Interpreter::RunFunction(clang::FunctionDecl const*, cling::Value*) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x72A2FE2: cling::Interpreter::EvaluateInternal(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, cling::CompilationOptions, cling::Value*, cling::Transaction**, unsigned long) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x72A3380: cling::Interpreter::process(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, cling::Value*, cling::Transaction**, bool) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x7366CC6: cling::MetaProcessor::process(llvm::StringRef, cling::Interpreter::CompilationResult&, cling::Value*, bool) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x71C477B: HandleInterpreterException(cling::MetaProcessor*, char const*, cling::Interpreter::CompilationResult&, cling::Value*) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x71DD059: TCling::ProcessLine(char const*, TInterpreter::EErrorCode*) (in /home/corentin/Installation/root/root_install/lib/libCling.so)

// ==2957==    by 0x4C0EDA5: TROOT::ProcessLine(char const*, int*) (in /home/corentin/Installation/root/root_install/lib/libCore.so)

// ==2957==    by 0x4C4CE6C: TObject::AppendPad(char const*) (in /home/corentin/Installation/root/root_install/lib/libCore.so)

// ==2957==    by 0x560AB82: TH1::Draw(char const*) (in /home/corentin/Installation/root/root_install/lib/libHist.so)

// ==2957==    by 0x553069D: void HFit::StoreAndDrawFitFunction<TH1>(TH1*, TF1*, ROOT::Fit::DataRange const&, bool, bool, char const*) (in /home/corentin/Installation/root/root_install/lib/libHist.so)

// ==2957==    by 0x5536FD0: TFitResultPtr HFit::Fit<TH1>(TH1*, TF1*, Foption_t&, ROOT::Math::MinimizerOptions const&, char const*, ROOT::Fit::DataRange&) (in /home/corentin/Installation/root/root_install/lib/libHist.so)

// ==2957==    by 0x552B992: ROOT::Fit::FitObject(TH1*, TF1*, Foption_t&, ROOT::Math::MinimizerOptions const&, char const*, ROOT::Fit::DataRange&) (in /home/corentin/Installation/root/root_install/lib/libHist.so)

// ==2957==    by 0x5601628: TH1::Fit(TF1*, char const*, char const*, double, double) (in /home/corentin/Installation/root/root_install/lib/libHist.so)

// ==2957==    by 0x14D39A: main (test.cpp:31)

// client stack range: [0x1FFEFEF000 0x1FFF000FFF] client SP: 0x1FFEFFCFF8

// valgrind stack range: [0x10034D3000 0x10035D2FFF] top usage: 18424 of 1048576





// Note: see also the FAQ in the source distribution.

// It contains workarounds to several common problems.

// In particular, if Valgrind aborted or crashed after

// identifying problems in your program, there's a good chance

// that fixing those problems will prevent Valgrind aborting or

// crashing, especially if it happened in m_mallocfree.c.



// If that doesn't help, please report this bug to: www.valgrind.org



// In the bug report, send all the above text, the valgrind

// version, and what OS and version you are using.  Thanks.

