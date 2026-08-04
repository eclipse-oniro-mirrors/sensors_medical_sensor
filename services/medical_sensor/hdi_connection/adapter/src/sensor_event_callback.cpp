/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#include "sensor_event_callback.h"
#include "hdi_connection.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "securec.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_HDI_ADAPTOR, "MedicalSensor_HdiConnection"
};
std::unique_ptr<HdiConnection> HdiConnection_ = std::make_unique<HdiConnection>();
}
int32_t SensorEventCallback::OnDataEvent(const HdfSensorEvents& event)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    SensorEvent sensorEvent = {
        .sensorTypeId = event.sensorId,
        .version = event.version,
        .timestamp = event.timestamp,
        .option = event.option,
        .mode = event.mode
    };
    sensorEvent.data = new uint8_t[SENSOR_DATA_LENGHT];
    if (memcpy_s(sensorEvent.data, SENSOR_DATA_LENGHT, event.data.data(),
        event.data.size()) != EOK) {
        HiLog::Error(LABEL, "%{public}s copy data failed", __func__);
        delete[] sensorEvent.data;
        return COPY_ERR;
    }
    sensorEvent.dataLen = static_cast<uint32_t>(event.data.size());
    DataCacheFunc reportDataCb_ = HdiConnection_->getReportDataCb();
    sptr<ReportDataCache> reportDataCallback_ = HdiConnection_->getReportDataCallback();
    if (reportDataCb_ == nullptr || reportDataCallback_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s reportDataCb_ cannot be null", __func__);
        return ERR_NO_INIT;
    }
    (void)(reportDataCallback_->*(reportDataCb_))(&sensorEvent, reportDataCallback_);
    ISensorHdiConnection::dataCondition_.notify_one();
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS