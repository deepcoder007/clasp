/*
    File: functor.cc
*/

/*
Copyright (c) 2014, Christian E. Schafmeister
 
CLASP is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
See directory 'clasp/licenses' for full details.
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
/* -^- */
#define DEBUG_LEVEL_FULL
#include <clasp/core/foundation.h>
#include <clasp/core/executables.h>
#include <clasp/core/lisp.h>
#include <clasp/core/str.h>
#include <clasp/core/symbolTable.h>
#include <clasp/core/standardObject.h>
#include <clasp/core/package.h>
//#include "debugger.h"
#include <clasp/core/iterator.h>
#include <clasp/core/designators.h>
#include <clasp/core/primitives.h>
#include <clasp/core/instance.h>
#include <clasp/core/vectorObjects.h>
#include <clasp/core/sourceFileInfo.h>
#include <clasp/core/activationFrame.h>
#include <clasp/core/lambdaListHandler.h>
//#i n c l u d e "environmentDependent.h"
#include <clasp/core/environment.h>
#include <clasp/core/evaluator.h>
// to avoid Generic to_object include headers here
#include <clasp/core/wrappers.h>



namespace core {
Functor_O::Functor_O(T_sp n) : name(n) {
  if (n.nilp()) {
    SIMPLE_ERROR(BF("Functoids must have a non-nil name"));
  }
}

string Functor_O::nameAsString() {
  if (this->name.nilp()) {
    return "Function-name(NIL)";
  } else if (Symbol_sp sname = this->name.asOrNull<Symbol_O>()) {
    stringstream ss;
    ss << "Function-name(";
    ss << sname->symbolNameAsString();
    ss << ")";
    return ss.str();
  } else if (Cons_sp cname = this->name.asOrNull<Cons_O>()) {
    stringstream ss;
    ss << "Function-name(setf ";
    ss << gc::As<Symbol_sp>(oCadr(cname))->symbolNameAsString();
    ss << ")";
    return ss.str();
  } else if (Str_sp strname = this->name.asOrNull<Str_O>()) {
    stringstream ss;
    ss << "Function-name(string-";
    ss << strname->get();
    ss << ")";
    return ss.str();
  }
  THROW_HARD_ERROR(BF("Cannot get name as string of Functoid"));
}



int Closure_O::sourceFileInfoHandle() const {
  return 0;
}

T_sp Closure_O::docstring() const {
  SIMPLE_ERROR(BF("Closure does not support docstring"));
}

List_sp Closure_O::declares() const {
  SIMPLE_ERROR(BF("Closure does not support declares"));
}

T_sp Closure_O::cleavir_ast() const {
  SIMPLE_ERROR(BF("Subclass of Closure must support cleavir_ast"));
}

T_sp Closure_O::setSourcePosInfo(T_sp sourceFile, size_t filePos, int lineno, int column)
{
  SIMPLE_ERROR(BF("Subclass of Closure must support this method"));
}

void Closure_O::setf_cleavir_ast(T_sp ast) {
  SIMPLE_ERROR(BF("Subclass of Closure must support setf_cleavir_ast"));
}
};


namespace core {

bool FunctionClosure_O::macroP() const {
  return this->kind == kw::_sym_macro;
}
int FunctionClosure_O::sourceFileInfoHandle() const {
  return this->_sourceFileInfoHandle;
};

size_t FunctionClosure_O::filePos() const {
  return this->_filePos;
}

int FunctionClosure_O::lineNumber() const {
  return this->_lineno;
}

int FunctionClosure_O::column() const {
  return this->_column;
}

T_sp FunctionClosure_O::setSourcePosInfo(T_sp sourceFile, size_t filePos, int lineno, int column) {
  SourceFileInfo_mv sfi = core__source_file_info(sourceFile);
  this->_sourceFileInfoHandle = gc::As<Fixnum_sp>(sfi.valueGet(1)).unsafe_fixnum();
  this->_filePos = filePos;
  this->_lineno = lineno;
  this->_column = column;
  SourcePosInfo_sp spi = SourcePosInfo_O::create(this->_sourceFileInfoHandle, filePos, lineno, column);
  return spi;
}

T_sp FunctionClosure_O::sourcePosInfo() const {
  SourcePosInfo_sp spi = SourcePosInfo_O::create(this->_sourceFileInfoHandle, this->_filePos, this->_lineno, this->_column);
  return spi;
}

T_sp BuiltinClosure_O::lambdaList() const {
  return this->_lambdaListHandler->lambdaList();
}

void BuiltinClosure_O::setf_lambda_list(T_sp lambda_list) {
  // Do nothing
}

LCC_RETURN BuiltinClosure_O::LISP_CALLING_CONVENTION() {
  IMPLEMENT_MEF(BF("Handle call to BuiltinClosure"));
};

InterpretedClosure_O::InterpretedClosure_O(T_sp fn, Symbol_sp k, LambdaListHandler_sp llh, List_sp dec, T_sp doc, T_sp e, List_sp c, SOURCE_INFO)
  : FunctionClosure_O(fn, k, e, SOURCE_INFO_PASS), _lambdaListHandler(llh), _declares(dec), _docstring(doc), _code(c) {
}

T_sp InterpretedClosure_O::lambdaList() const {
  return this->lambdaListHandler()->lambdaList();
}

void InterpretedClosure_O::setf_lambda_list(T_sp lambda_list) {
  // Do nothing - setting the lambdaListHandler is all that's needed
}

LCC_RETURN InterpretedClosure_O::LISP_CALLING_CONVENTION() {
  ValueEnvironment_sp newValueEnvironment = ValueEnvironment_O::createForLambdaListHandler(this->_lambdaListHandler, this->closedEnvironment);
  ValueEnvironmentDynamicScopeManager scope(newValueEnvironment);
  InvocationHistoryFrame _frame(this->asSmartPtr(), lcc_arglist);
  lambdaListHandler_createBindings(this->asSmartPtr(), this->_lambdaListHandler, scope, LCC_PASS_ARGS);
  ValueFrame_sp newActivationFrame = gc::As<ValueFrame_sp>(newValueEnvironment->getActivationFrame());
  VectorObjects_sp debuggingInfo = _lambdaListHandler->namesOfLexicalVariablesForDebugging();
  newActivationFrame->attachDebuggingInfo(debuggingInfo);
  //        InvocationHistoryFrame _frame(this,newActivationFrame);
  _frame.setActivationFrame(newActivationFrame);
#if 0
  if (_sym_STARdebugInterpretedClosureSTAR->symbolValue().notnilp()) {
    printf("%s:%d Entering InterpretedClosure   source file = %s  lineno=%d\n", __FILE__, __LINE__, _frame.sourcePathName().c_str(), _frame.lineno());
  }
#endif
  return eval::sp_progn(this->_code, newValueEnvironment).as_return_type();
};


};

namespace core {

core::T_sp CompiledClosure_O::lambdaList() const {
  return this->_lambdaList;
}

void CompiledClosure_O::setf_lambda_list(core::T_sp lambda_list) {
  this->_lambdaList = lambda_list;
}


T_sp InstanceCreator_O::allocate() {
  GC_ALLOCATE(Instance_O, output);
  return output;
};


LCC_RETURN InstanceClosure_O::LISP_CALLING_CONVENTION() {
// Copy the arguments passed in registers into the multiple_values array and those
// will be processed by the generic function
#ifdef _DEBUG_BUILD
  VaList_S saved_args(*reinterpret_cast<VaList_S *>(untag_valist(lcc_arglist)));
#endif
  VaList_sp gfargs((gc::Tagged)lcc_arglist);
  //  LCC_SKIP_ARG(gfargs);
  return (this->entryPoint)(this->instance, gfargs);
}



};