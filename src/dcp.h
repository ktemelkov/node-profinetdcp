#ifndef __DCP_H
#define __DCP_H

Napi::Value DcpIdentify(const Napi::CallbackInfo& info);
Napi::Value DcpGet(const Napi::CallbackInfo& info);
Napi::Value DcpSet(const Napi::CallbackInfo& info);

#endif //__DCP_H