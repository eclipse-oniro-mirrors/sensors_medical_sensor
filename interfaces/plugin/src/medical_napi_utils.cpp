/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "medical_napi_utils.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "hilog/log.h"

using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0xD002786, "AfeJsAPI"};
bool IsMatchType(napi_value value, napi_valuetype type, napi_env env)
{
    napi_valuetype paramType;
    napi_typeof(env, value, &paramType);
    if (paramType != type) {
        HiLog::Error(LABEL, "%{public}s  failed!", __func__);
        return false;
    }
    return true;
}

napi_value GetNapiInt32(int32_t number, napi_env env)
{
    napi_value value;
    napi_create_int32(env, number, &value);
    return value;
}

napi_value NapiGetNamedProperty(napi_value jsonObject, std::string name, napi_env env)
{
    napi_value value;
    napi_get_named_property(env, jsonObject, name.c_str(), &value);
    return value;
}

int32_t GetCppInt32(napi_value value, napi_env env)
{
    int32_t number;
    napi_get_value_int32(env, value, &number);
    return number;
}

int64_t GetCppInt64(napi_value value, napi_env env)
{
    int64_t number;
    napi_get_value_int64(env, value, &number);
    return number;
}

bool GetCppBool(napi_value value, napi_env env)
{
    bool number;
    napi_get_value_bool(env, value, &number);
    return number;
}

napi_value GetUndefined(napi_env env)
{
    napi_value value;
    napi_get_undefined(env, &value);
    return value;
}

std::map<int32_t, std::vector<std::string>> g_sensorAttributeList = {
    { AFE_TYPE_ID_NONE, { "dataArray" } },
    { AFE_TYPE_ID_PHOTOPLETHYSMOGRAPH, { "dataArray" } },
};

void EmitAsyncCallbackWork(AsyncCallbackInfo *asyncCallbackInfo)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (asyncCallbackInfo == nullptr) {
        HiLog::Error(LABEL, "%{public}s asyncCallbackInfo is null!", __func__);
        return;
    }
    napi_value resourceName;
    if (napi_create_string_utf8(asyncCallbackInfo->env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName) != napi_ok) {
        HiLog::Error(LABEL, "%{public}s create string utf8 failed", __func__);
        return;
    }
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            HiLog::Debug(LABEL, "%{public}s napi_create_async_work in", __func__);
            AsyncCallbackInfo *asyncCallbackInfo = reinterpret_cast<AsyncCallbackInfo *>(data);
            napi_value callback;
            napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
            napi_value callResult = nullptr;
            napi_value result[2] = {0};
            if (asyncCallbackInfo->status < 0) {
                HiLog::Debug(LABEL, "%{public}s napi_create_async_work < 0 in", __func__);
                napi_value code = nullptr;
                napi_value message = nullptr;
                napi_create_string_utf8(env, "-1", NAPI_AUTO_LENGTH, &code);
                napi_create_string_utf8(env, "failed", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, code, message, &result[0]);
                napi_get_undefined(env, &result[1]);
            } else if (asyncCallbackInfo->status == 0) {
                napi_get_undefined(env, &result[1]);
                napi_get_undefined(env, &result[0]);
            }
            napi_call_function(env, nullptr, callback, 2, result, &callResult);
            napi_delete_reference(env, asyncCallbackInfo->callback[0]);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HiLog::Debug(LABEL, "%{public}s napi_create_async_work left", __func__);
        },
        asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
    HiLog::Debug(LABEL, "%{public}s end", __func__);
}

void EmitUvEventLoop(AsyncCallbackInfo *asyncCallbackInfo)
{
    uv_loop_s *loop(nullptr);
    HiLog::Debug(LABEL, "%{public}s env: %{public}p", __func__, asyncCallbackInfo->env);
    napi_get_uv_event_loop(asyncCallbackInfo->env, &loop);
    if (loop == nullptr) {
        HiLog::Error(LABEL, "%{public}s loop is null", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        HiLog::Error(LABEL, "%{public}s work is null", __func__);
        return;
    }

    work->data = reinterpret_cast<void *>(asyncCallbackInfo);
    uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        AsyncCallbackInfo *asyncCallbackInfo = reinterpret_cast<AsyncCallbackInfo *>(work->data);
        if (asyncCallbackInfo == nullptr) {
            HiLog::Error(LABEL, "%{public}s asyncCallbackInfo is null", __func__);
            return;
        }
        napi_env env = asyncCallbackInfo->env;
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        if (asyncCallbackInfo->callback[0] == nullptr) {
            HiLog::Error(LABEL, "%{public}s callback is null", __func__);
            return;
        }
        napi_value callback;
        napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
        napi_value callResult = nullptr;
        napi_value result[2] = {0};
        if (asyncCallbackInfo->status < 0) {
            HiLog::Debug(LABEL, "%{public}s status < 0 in", __func__);
            napi_value code = nullptr;
            napi_value message = nullptr;
            napi_create_string_utf8(env, "-1", NAPI_AUTO_LENGTH, &code);
            napi_create_string_utf8(env, "failed", NAPI_AUTO_LENGTH, &message);
            napi_create_error(env, code, message, &result[0]);
            napi_get_undefined(env, &result[1]); 
        } else {
            int32_t sensorTypeId = asyncCallbackInfo->sensorTypeId;
            if (g_sensorAttributeList.count(sensorTypeId) == 0) {
                HiLog::Error(LABEL, "%{public}s count of sensorTypeId is zero", __func__);
                return;
            }

            std::vector<std::string> sensorAttribute = g_sensorAttributeList[sensorTypeId];
            napi_create_object(env, &result[1]);
            for (size_t i = 0; i < sensorAttribute.size(); i++) {
                napi_value message = nullptr;
                napi_create_array(env, &message);
                for (size_t j = 0; j < asyncCallbackInfo->sensorDataLength; j++) {
                    napi_value num;
                    napi_create_uint32(env, asyncCallbackInfo->sensorData[j], &num);
                    napi_set_element(env, message, j, num);
                }
                napi_set_named_property(env, result[1], sensorAttribute[i].c_str(), message);
                HiLog::Error(LABEL, "%{public}s sensorData[0]=%{public}d", __func__, *(uint32_t *)message);
            }
            napi_get_undefined(env, &result[0]);
        }
        napi_call_function(env, undefined, callback, 2, result, &callResult);
        if (asyncCallbackInfo->status != 1) {
            napi_delete_reference(env, asyncCallbackInfo->callback[0]);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        delete work;
        work = nullptr;
    });
}
