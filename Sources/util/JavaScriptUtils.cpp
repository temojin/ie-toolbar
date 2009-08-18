/**
* Facebook Internet Explorer Toolbar Software License 
* Copyright (c) 2009 Facebook, Inc. 
*
* Permission is hereby granted, free of charge, to any person or organization
* obtaining a copy of the software and accompanying documentation covered by
* this license (which, together with any graphical images included with such
* software, are collectively referred to below as the "Software") to (a) use,
* reproduce, display, distribute, execute, and transmit the Software, (b)
* prepare derivative works of the Software (excluding any graphical images
* included with the Software, which may not be modified or altered), and (c)
* permit third-parties to whom the Software is furnished to do so, all
* subject to the following:
*
* The copyright notices in the Software and this entire statement, including
* the above license grant, this restriction and the following disclaimer,
* must be included in all copies of the Software, in whole or in part, and
* all derivative works of the Software, unless such copies or derivative
* works are solely in the form of machine-executable object code generated by
* a source language processor.  
*
* Facebook, Inc. retains ownership of the Software and all associated
* intellectual property rights.  All rights not expressly granted in this
* license are reserved by Facebook, Inc.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/ 

#include "StdAfx.h"
#include "JavaScriptUtils.h"

#include <boost/bind.hpp>

#include "ScopeGuard.h"

namespace facebook {

// helper functions 

/**
 * Trying to locate function's DISPID in the script  
 *
 * @param funcName name of function(by ref)
 * @param script to llok for function(by ref)
 *
 * return script DISPID
 */
DISPID getScriptDISPID(const facebook::String& funcName, CComPtr<IDispatch>& script) {
  DISPID dispID = 0;
  _bstr_t bstrVal(funcName.c_str()); 
  script->GetIDsOfNames(IID_NULL, &bstrVal.GetBSTR() ,1,
    LOCALE_SYSTEM_DEFAULT,&dispID);

  //if FAILED(getIDsOfNamesRes) {
  //  _com_raise_error(getIDsOfNamesRes);
  //}
  return dispID;
}

/**
 * Prepare params to call function  
 *
 * @param paramArray - input array of params (by ref)
 * @param dispParams - prepared params for call (by ref)
 *
 * return void
 */
void fillDispParams(const CStringArray& paramArray, /*out*/ DISPPARAMS& dispParams) {
  const int arraySize = paramArray.GetSize();
  memset(&dispParams, 0, sizeof dispParams);
  dispParams.cArgs      = arraySize;
  dispParams.rgvarg     = new VARIANT[dispParams.cArgs];
  dispParams.cNamedArgs = 0;

  for(int i = 0; i < arraySize; i++) {
    CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i);
            // back reading
    bstr.CopyTo(&dispParams.rgvarg[i].bstrVal);
    dispParams.rgvarg[i].vt = VT_BSTR;
  }
}

/**
 * Clears params   
 *
 * @param dispParams - params to clear (by ref)
 *
 * return void
 */
void clearDispParams(/*in*/ DISPPARAMS& dispParams) {
  delete [] dispParams.rgvarg;
  memset(&dispParams, 0, sizeof dispParams);
}

/**
 * Invokes JS function with params  
 *
 * @param script that contain function (by ref)
 * @param dispParams - params to pass to function (by ref)
 * @param dispID - id of function to call (by ref)
 *
 * return void
 */
HRESULT invokeJavaScriptFunc(CComPtr<IDispatch>& script, DISPPARAMS& dispParams, const DISPID dispID , CComVariant &vaResult) {
  EXCEPINFO excepInfo;
  memset(&excepInfo, 0, sizeof excepInfo);
  UINT nArgErr = (UINT)-1;      // initialize to invalid arg

  HRESULT hr = script->Invoke(dispID, IID_NULL, 0,
    DISPATCH_METHOD, &dispParams, &vaResult, &excepInfo, &nArgErr);

  return hr;
}

CComVariant callJSFunc(CComPtr<IHTMLDocument2>& htmlDoc, 
      const facebook::String& funcName, const CStringArray& paramArray) {

  // get html scripts 
  CComPtr<IDispatch> script;
  HRESULT getScriptRes = htmlDoc->get_Script(&script);
  if FAILED(getScriptRes) {
    _com_raise_error(getScriptRes);
  }

  // get disp id of script object
  DISPID dispID;
  dispID = getScriptDISPID(funcName, script);

  // initialize parameters on construction
  // clear on destruction

  if (dispID == -1)
    return CComVariant();
  DISPPARAMS dispParams;
  using namespace boost;
  facebook::ScopeGuard dispParms(
  bind(fillDispParams, cref(paramArray), ref(dispParams)),
  bind(clearDispParams, ref(dispParams)));

  CComVariant vaResult;
  
  invokeJavaScriptFunc(script, dispParams, dispID, vaResult);
  vaResult.lVal;
  return vaResult;
}

} // !namespace facebook
