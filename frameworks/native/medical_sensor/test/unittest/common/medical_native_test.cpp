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
        HiLog::Info(LABEL, "%{public}s sensorTypeId = %{public}d, data->sensorid = %{public}u", __func__, events[i].sensorTypeId, test->afeId_);
        if (events[i].sensorTypeId == (int32_t)test->afeId_) {
            HiLog::Info(LABEL, "%{public}s got report data, set test->dataReport_ = true", __func__);
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
    int32_t ret = afeDataChannel_->CreateSensorDataChannel(HandleEvent, this);
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
 * @tc.name: GetSensorList_001
 * @tc.desc: get afe list
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, GetSensorList_001, TestSize.Level0)
{
    bool ret = afesList_.empty();
    HiLog::Info(LABEL, "GetSensorList_001,count : %{public}d!", int{ afesList_.size() });
    ASSERT_EQ(ret, false);
}

/*
 * @tc.name: GetSensorList_002
 * @tc.desc: Judge afe info
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, GetSensorList_002, TestSize.Level0)
{
    for (const auto &it : afesList_) {
        ASSERT_EQ(it.GetSensorId() == 0 || it.GetSensorId() == 129, true);
        ASSERT_EQ(it.GetName().empty(), false);
        ASSERT_EQ(it.GetVendor().empty(), false);
    }
    HiLog::Info(LABEL, "GetSensorList_002");
}

/*
 * @tc.name: SensorOperation_001
 * @tc.desc: disable afe
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
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    ret = afeServiceClient_->DisableSensor(afeId);
    HiLog::Info(LABEL, "DisableSensor ret is : %{public}d", ret);
    ASSERT_EQ(ret, ERR_OK);

    ASSERT_EQ(dataReport_, true);
}

/*
 * @tc.name: ClientAfeDataChannel_001
 * @tc.desc: do client afe send data channel close
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, ClientAfeDataChannel_001, TestSize.Level0)
{
    int32_t ret = afeDataChannel_->GetSendDataFd();
    HiLog::Info(LABEL, "ClientAfeDataChannel_001,ret : %{public}d!", ret);
    ASSERT_EQ(ret, -1);
}

/*
 * @tc.name: ClientAfeDataChannel_002
 * @tc.desc: judge channel fd when create channel
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, ClientAfeDataChannel_002, TestSize.Level0)
{
    int32_t ret = afeDataChannel_->GetReceiveDataFd();
    ASSERT_EQ((ret >= 0), true);
}

/*
 * @tc.name: ClientAfeDataChannel_003
 * @tc.desc: Judge read thread status when Destroy Channel
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, ClientAfeDataChannel_003, TestSize.Level0)
{
    int32_t ret = afeDataChannel_->DestroySensorDataChannel();
    ASSERT_EQ(ret, ERR_OK);
}

/*
 * @tc.name: ClientAfeDataChannel_004
 * @tc.desc: Destroy ClientAfeDataChannel
 * @tc.type: FUNC
 */
HWTEST_F(AfeNativeTest, ClientAfeDataChannel_004, TestSize.Level0)
{
    int32_t ret = afeDataChannel_->DestroySensorDataChannel();
    ASSERT_EQ(ret, ERR_OK);
    ret = afeDataChannel_->GetSendDataFd();
    ASSERT_EQ(ret, -1);
    ret = afeDataChannel_->GetReceiveDataFd();
    ASSERT_EQ(ret, -1);
}
}  // namespace Sensors
}  // namespace OHOS
