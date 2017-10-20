
// Nothing for now

#include <clasp/core/core.h>
#include <clasp/core/object.h>
#include <clasp/core/array.h>
#include <clasp/core/instance.h>
#include <clasp/core/accessor.h>
#include <clasp/core/wrappedPointer.h>
#include <clasp/core/funcallableInstance.h>
#include <clasp/gctools/gcStack.h>
#include <clasp/llvmo/intrinsics.h>

#if 0 // DEBUGGING
#define NO_UNWIND_BEGIN_BUILTINS() NO_UNWIND_BEGIN()
#define NO_UNWIND_END_BUILTINS() NO_UNWIND_END()
#define ENSURE_VALID_OBJECT_BUILTINS(x) ENSURE_VALID_OBJECT(x)
#else
#define NO_UNWIND_BEGIN_BUILTINS()
#define NO_UNWIND_END_BUILTINS()
#define ENSURE_VALID_OBJECT_BUILTINS(x) x
#endif

#define LINKAGE __attribute__ ((visibility ("default")))

#define BUILTIN_ATTRIBUTES __attribute__((always_inline))

extern "C" {

BUILTIN_ATTRIBUTES int foobar(int x) {return x*x*x*x;}

};

extern "C" {

BUILTIN_ATTRIBUTES void newTmv(core::T_mv *sharedP)
{
  new (sharedP) core::T_mv();
}

BUILTIN_ATTRIBUTES void cc_rewind_va_list(core::T_O* tagged_closure, va_list va_args, size_t* nargsP, void** register_save_areaP)
{NO_UNWIND_BEGIN_BUILTINS();
#if 0
  if (core::debug_InvocationHistoryFrame==3) {
    printf("%s:%d cc_rewind_va_list     va_args=%p     nargsP = %p      register_save_areaP = %p\n", __FILE__, __LINE__, va_args, nargsP, register_save_areaP );
  }
#endif
  LCC_REWIND_VA_LIST(va_args,register_save_areaP);
  *nargsP = (uintptr_t)register_save_areaP[1];
  NO_UNWIND_END_BUILTINS();
}

BUILTIN_ATTRIBUTES
core::T_O *va_symbolFunction(core::T_O *symP) {
  core::Symbol_sp sym((gctools::Tagged)symP);
  unlikely_if (!sym->fboundp()) intrinsic_error(llvmo::noFunctionBoundToSymbol, sym);
  core::Function_sp func((gc::Tagged)(sym)->_Function.theObject);
  return func.raw_();
}


#if 0
BUILTIN_ATTRIBUTES core::T_sp *symbolValueReference(core::T_sp *symbolP)
{
  core::Symbol_sp sym((gctools::Tagged)ENSURE_VALID_OBJECT_BUILTINS(symbolP->raw_()));
  return sym->valueReference();
}
#endif


BUILTIN_ATTRIBUTES core::T_sp *lexicalValueReference(int depth, int index, core::ActivationFrame_O *frameP)
{
  core::ActivationFrame_sp af((gctools::Tagged)frameP);
  return const_cast<core::T_sp *>(&core::value_frame_lookup_reference(af, depth, index));
}

BUILTIN_ATTRIBUTES core::T_sp *registerReference(core::T_sp* register_)
{
  return register_;
}


#if 0
BUILTIN_ATTRIBUTES void sp_lexicalValueRead(core::T_sp *resultP, int depth, int index, core::ActivationFrame_sp *renvP)
{
  (*resultP) = core::value_frame_lookup_reference(*renvP,depth,index);
}
BUILTIN_ATTRIBUTES void mv_lexicalValueRead(core::T_mv *resultP, int depth, int index, core::ActivationFrame_sp *renvP)
{
  (*resultP) = core::value_frame_lookup_reference(*renvP,depth,index);
}
#endif

// The following two are only valid for non-simple arrays. Be careful!
BUILTIN_ATTRIBUTES core::T_O* cc_realArrayDisplacement(core::T_O* tarray) {
  core::MDArray_O* array = reinterpret_cast<core::MDArray_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->realDisplacedTo().raw_();
}
BUILTIN_ATTRIBUTES size_t cc_realArrayDisplacedIndexOffset(core::T_O* tarray) {
  core::MDArray_O* array = reinterpret_cast<core::MDArray_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->displacedIndexOffset();
}

BUILTIN_ATTRIBUTES size_t cc_arrayTotalSize(core::T_O* tarray) {
  core::MDArray_O* array = reinterpret_cast<core::MDArray_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->arrayTotalSize();
}

BUILTIN_ATTRIBUTES size_t cc_arrayRank(core::T_O* tarray) {
  core::MDArray_O* array = reinterpret_cast<core::MDArray_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->rank();
}

BUILTIN_ATTRIBUTES size_t cc_arrayDimension(core::T_O* tarray, size_t axis) {
  core::MDArray_O* array = reinterpret_cast<core::MDArray_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->arrayDimension(axis);
}

BUILTIN_ATTRIBUTES uint cc_simpleBitVectorAref(core::T_O* tarray, size_t index) {
  core::SimpleBitVector_O* array = reinterpret_cast<core::SimpleBitVector_O*>(gctools::untag_general<core::T_O*>(tarray));
  return array->testBit(index);
}

BUILTIN_ATTRIBUTES void cc_simpleBitVectorAset(core::T_O* tarray, size_t index, uint v) {
  core::SimpleBitVector_O* array = reinterpret_cast<core::SimpleBitVector_O*>(gctools::untag_general<core::T_O*>(tarray));
  array->setBit(index, v);
}

BUILTIN_ATTRIBUTES core::T_O* invisible_makeValueFrameSetParent(core::T_O* parent) {
  return parent;
}

BUILTIN_ATTRIBUTES core::T_O* invisible_makeValueFrameSetParentFromClosure(core::T_O* closureRaw) {
  if (closureRaw!=NULL) {
    core::Closure_O* closureP = reinterpret_cast<core::Closure_O*>(gc::untag_general<core::T_O*>(closureRaw));
    core::T_sp activationFrame = closureP->closedEnvironment();
    return activationFrame.raw_(); // >rawRef_() = closureRaw; //  = activationFrame;
  } else {
    return _Nil<core::T_O>().raw_();
  }
}


/*! Return i32 1 if (valP) is != unbound 0 if it is */
BUILTIN_ATTRIBUTES int isBound(core::T_O *valP)
{
  return gctools::tagged_unboundp<core::T_O*>(valP) ? 0 : 1;
}

/*! Return i32 1 if (valP) is != nil 0 if it is */
BUILTIN_ATTRIBUTES int isTrue(core::T_O* valP)
{
  return gctools::tagged_nilp<core::T_O*>(valP) ? 0 : 1;
}

/*! Return i32 1 if (valP) is != nil 0 if it is */
BUILTIN_ATTRIBUTES core::T_O* valueOrNilIfZero(gctools::return_type val) {
  return val.nvals ? val.ret0[0] : _Nil<core::T_O>().raw_();
}

BUILTIN_ATTRIBUTES core::T_O** activationFrameReferenceFromClosure(core::T_O* closureRaw)
{
  ASSERT(closureRaw);
  if (closureRaw!=NULL) {
    core::ClosureWithFrame_sp closure = core::ClosureWithFrame_sp((gctools::Tagged)closureRaw);
    return &closure->_closedEnvironment.rawRef_();
  }
  return NULL;
}

BUILTIN_ATTRIBUTES void* cc_vaslist_va_list_address(core::T_O* vaslist)
{
  return &(gctools::untag_valist(vaslist)->_Args);
};

BUILTIN_ATTRIBUTES size_t* cc_vaslist_remaining_nargs_address(core::Vaslist* vaslist)
{
  return &(gctools::untag_valist(vaslist)->_remaining_nargs);
};


BUILTIN_ATTRIBUTES core::T_O *cc_fetch(core::T_O *tagged_closure, std::size_t idx)
{
  gctools::smart_ptr<core::ClosureWithSlots_O> c = gctools::smart_ptr<core::ClosureWithSlots_O>((gc::Tagged)tagged_closure);
  return (*c)[idx].raw_();
}

BUILTIN_ATTRIBUTES core::T_O *cc_readCell(core::T_O *cell)
{
  core::Cons_O* cp = reinterpret_cast<core::Cons_O*>(gctools::untag_cons(cell));
  return cp->_Car.raw_();
}



BUILTIN_ATTRIBUTES core::T_O* cc_dispatch_slot_reader_index(size_t index, core::T_O* tinstance) {
  core::Instance_sp instance((gctools::Tagged)tinstance);
  core::T_sp value = low_level_instanceRef(instance->_Rack,index);
  return value.raw_();
}

BUILTIN_ATTRIBUTES core::T_O* cc_dispatch_slot_reader_cons(core::T_O* toptinfo) {
  core::SimpleVector_sp optinfo((gctools::Tagged)toptinfo);
  core::Cons_sp cons = gc::As_unsafe<core::Cons_sp>((*optinfo)[OPTIMIZED_SLOT_INDEX_INDEX]);
  core::T_sp value = CONS_CAR(cons);
  return value.raw_();
}

BUILTIN_ATTRIBUTES gctools::return_type cc_bound_or_error(core::T_O* toptimized_slot_reader, core::T_O* tinstance, core::T_O* tvalue) {
  core::T_sp value((gctools::Tagged)tvalue);
  if (value.unboundp()) {
    core::Instance_sp instance((gctools::Tagged)tinstance);
    core::T_sp optimized_slot_info((gctools::Tagged)toptimized_slot_reader);
    return llvmo::intrinsic_slot_unbound(optimized_slot_info,instance).as_return_type();
  }
  return value.as_return_type();
}

BUILTIN_ATTRIBUTES gctools::return_type cc_dispatch_slot_writer_index(core::T_O* tvalue, size_t index, core::T_O* tinstance) {
  core::T_sp value((gctools::Tagged)tvalue);
  core::Instance_sp instance((gctools::Tagged)tinstance);
  low_level_instanceSet(instance->_Rack,index,value);
  return value.as_return_type();
}

BUILTIN_ATTRIBUTES gctools::return_type cc_dispatch_slot_writer_cons(core::T_O* tvalue, core::T_O* toptinfo) {
  core::SimpleVector_sp optinfo((gctools::Tagged)toptinfo);
  core::Cons_sp cons = gc::As_unsafe<core::Cons_sp>((*optinfo)[OPTIMIZED_SLOT_INDEX_INDEX]);
  core::T_sp value((gctools::Tagged)tvalue);
  CONS_CAR(cons) = value;
  return value.as_return_type();
}


BUILTIN_ATTRIBUTES void cc_vaslist_end(core::T_O* tvaslist) {
  core::VaList_sp vaslist((gctools::Tagged)tvaslist);
  va_end(vaslist->_Args);
}

BUILTIN_ATTRIBUTES gctools::return_type cc_dispatch_effective_method(core::T_O* teffective_method, core::T_O* tgf, core::T_O* tgf_args_valist_s) {
  core::Function_sp effective_method((gctools::Tagged)teffective_method);
//  core::T_sp gf((gctools::Tagged)tgf);
  core::T_sp gf_args((gctools::Tagged)tgf_args_valist_s);
//  printf("%s:%d  Invoking effective-method %s with arguments %s\n", __FILE__, __LINE__,
  // Arguments are .method-args. .next-methods.

  return (*effective_method).entry(LCC_PASS_ARGS2_ELLIPSIS(teffective_method,gf_args.raw_(),_Nil<core::T_O>().raw_()));
#if 0
  return (apply_method0(effective_method.raw_(),
                        gf_args.raw_(),
                        _Nil<core::T_O>().raw_(),
                        gf_args.raw_());
#endif
}

};
