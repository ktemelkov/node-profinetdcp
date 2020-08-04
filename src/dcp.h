#ifndef __DCP_H
#define __DCP_H

Napi::Value DcpIdentify(Napi::CallbackInfo& info);
Napi::Value DcpGet(Napi::CallbackInfo& info);
Napi::Value DcpSet(Napi::CallbackInfo& info);

#endif //__DCP_H