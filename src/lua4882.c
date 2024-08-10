/*
--------------------------------------------------------------------------------
MIT License

luadaqmx - Copyright (c) 2024 Kritzel Kratzel.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

--------------------------------------------------------------------------------

National Instruments  NI-488.2 API
Copyright (c) National Instruments 2001-2007. All Rights Reserved.     
https://www.ni.com/en/about-ni/legal/software-license-agreement.html

--------------------------------------------------------------------------------
*/

#ifdef _WINDLL
#include <windows.h>
#define DLL __declspec(dllexport)
#else
#define DLL //empty
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <ni4882.h>

#define TRUE 1
#define FALSE 0
#define LUA4882_VERSION "lua4882 v0.1"

#ifdef _WINDLL

static const char* decodeErrorCode(int err) {
  // Error messages corresponding to global variable iberr
  const char *msg;
  switch(err) {
  case EDVR:
    msg = "EDVR: System error";
    break;
  case ECIC:
    msg = "ECIC: Function requires GPIB board to be CIC";
    break;
  case ENOL:
    msg = "ENOL: Write function detected no Listeners";
    break;
  case EADR:
    msg = "EADR: Interface board not addressed correctly";
    break;
  case EARG:
    msg = "EARG: Invalid argument to function call";
    break;
  case ESAC:
    msg = "ESAC: Function requires GPIB board to be SAC";
    break;
  case EABO:
    msg = "EABO: I/O operation aborted";
    break;
  case ENEB:
    msg = "ENEB: Non-existent interface board";
    break;
  case EDMA:
    msg = "EDMA: Error performing DMA";
    break;
  case EOIP:
    msg = "EOIP: I/O operation started before previous operation completed";
    break;
  case ECAP:
    msg = "ECAP: No capability for intended operation";
    break;
  case EFSO:
    msg = "EFSO: File system operation error";
    break;
  case EBUS:
    msg = "EBUS: Command error during device call";
    break;
  case ESTB:
    msg = "ESTB: Serial poll status byte lost";
    break;
  case ESRQ:
    msg = "ESRQ: SRQ remains asserted";
    break;
  case ETAB:
    msg = "ETAB: The return buffer is full";
    break;
  case ELCK:
    msg = "ELCK: Address or board is locked";
    break;
  case EARM:
    msg = "EARM: The ibnotify Callback failed to rearm";
    break;
  case EHDL:
    msg = "EHDL: The input handle is invalid";
    break;
  case EWIP:
    msg = "EWIP: Wait already in progress on input ud";
    break;
  case ERST:
    msg = "ERST: The event notification was cancelled due to a reset of the interface";
    break;
  case EPWR:
    msg = "EPWR: The system or board has lost power or gone to standby";
    break;
  default:
    msg = "E???: Undocumented error";
    break;
  }
  return msg;
}

static int lua4882_ibdev(lua_State *L) {
  // Open and initialize a device descriptor
  // int ibdev (int BdIndx, int pad, int sad, int tmo, int eot, int eos)

  // Check number of arguments
  if (lua_gettop(L) != 6) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int idx = (int)luaL_checkinteger(L,1);
  int pad = (int)luaL_checkinteger(L,2);
  int sad = (int)luaL_checkinteger(L,3);
  int tmo = (int)luaL_checkinteger(L,4);
  int eot = (int)luaL_checkinteger(L,5);
  int eos = (int)luaL_checkinteger(L,6);
  // call C-function
  int handle = ibdev(idx, pad, sad, tmo, eot, eos);
  // Error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);		// no handle or descriptor
    lua_pushstring(L,decodeErrorCode(Iberr())); // errmsg
  }
  else {
    // OK
    lua_pushinteger(L,handle);	// device descriptor
    lua_pushnil(L);		// no errmsg
  }
  return 2;
}

#else
// FIXME - non-_WINDLL not yet implemented
#endif

static const struct luaL_Reg lua4882_metamethods [] = {
  {"__call", lua4882_ibdev},
  {NULL, NULL}
};

static const struct luaL_Reg lua4882_funcs [] = {
  {"ibdev", lua4882_ibdev},
  {NULL, NULL}
};

DLL int luaopen_lua4882(lua_State *L){
  luaL_newlib(L, lua4882_funcs);
  luaL_newlib(L, lua4882_metamethods);
  lua_setmetatable(L, -2);
  lua_pushliteral(L,LUA4882_VERSION);
  lua_setfield(L,-2,"_VERSION");
  return 1;
}


