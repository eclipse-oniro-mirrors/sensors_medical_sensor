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
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "medical_native_type.h"
#include "medical_data_channel.h"
#include "medical_service_client.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_TEST, "AfeNativeTest" };
}

class AfeNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static void HandleEvent(struct SensorEvent *events, int32_t num, void *data);
    std::vector<MedicalSensor> afesList_;
    uint32_t afeId_;
    sptr<MedicalSensorDataChannel> afeDataChannel_;
    std::unique_ptr<MedicalSensorServiceClient> afeServiceClient_;
    bool dataReport_;
};
void AfeNativeTest::HandleEvent(struct SensorEvent *events, int32_t num, void *data)
{
    ASSERT_NE(events, nullptr);
    HiLog::Info(LABEL, "%{public}s start,num : %{public}d", __func__, num);
    AfeNativeTest *test = reinterpret_cast<AfeNativeTest *>(data);
    ASSERT_NE(test, nullptr);
    for (int32_t i = 0; i < num; i++) {
        HiLog::Info(LABEL, "%{public}s: sensorId : %{public}d, afeid: %{public}d", __func__, events[i].sensorTypeId, test->afeId_);
        if (events[i].sensorTypeId == (int32_t)test->afeId_) {
            test->dataReport_ = true;
        }
    }
    return;
}
void AfeNativeTest::SetUpTestCase()
{}

void AfeNativeTest::TearDownTestCase()
{}

void AfeNativeTest::SetUp()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    afeDataChannel_ = new (std::nothrow) MedicalSensorDataChannel();
    ASSERT_NE(afeDataChannel_, nullptr);
    afeServiceClient_ = std::make_unique<MedicalSensorServiceClient>();
    ASSERT_NE(afeServiceClient_, nullptr);
    afesList_ = afeServiceClient_->GetSensorList();
    ASSERT_NE(afesList_.size(), 0UL);
    afeId_ = afesList_[0].GetSensorId();
    auto ret = afeDataChannel_->CreateSensorDataChannel(HandleEvent, this);
    HiLog::Info(LABEL, "CreateSensorDataChannel ret is : %{public}d", ret);
    ASSERT_EQ(ret, ERR_OK);
    afeServiceClient_->TransferDataChannel(afeDataChannel_);
    dataReport_ = false;
}

void AfeNativeTest::TearDown()
{
    afeServiceClient_->DestroyDataChannel();
    afeDataChannel_->DestroySensorDataChannel();
}

/*
 * @tc.name: SensorOperation_001
 * @tc.desc: enable afe
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, SensorOperation_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "SensorOperation_001 begin");
    uint64_t samplingPeriodNs = 10000000;
    uint64_t maxReportDelayNs = 0;
    auto afeId = afesList_[0].GetSensorId();
    auto ret = afeServiceClient_->EnableSensor(afeId, samplingPeriodNs, maxReportDelayNs);
    HiLog::Info(LABEL, "EnableSensor ret is : %{public}d, afeId: %{public}d", ret, afeId);
    ASSERT_EQ(ret, ERR_OK);
    dataReport_ = false;
    // wait evennt: need about 5s from afe enable to data report
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    ASSERT_EQ(dataReport_, true);

    ret = afeServiceClient_->DisableSensor(afeId);
    HiLog::Info(LABEL, "DisableSensor ret is : %{public}d", ret);
    ASSERT_EQ(ret, ERR_OK);
}
}  // namespace Sensors
}  // namespace OHOS
