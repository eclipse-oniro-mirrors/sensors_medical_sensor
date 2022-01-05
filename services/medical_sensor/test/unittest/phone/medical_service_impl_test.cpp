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

#include "report_data_cache.h"
#include "medical_native_type.h"
#include "medical_data_channel.h"
#include "sensor_if.h"
#include "medical_service_client.h"
#include "medical_service_impl.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::SENSOR_TEST, "AfeServiceImplTest" };
}  // namespace

class AfeServiceImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    MedicalSensorServiceImpl &afeServiceImpl_ = MedicalSensorServiceImpl::GetInstance();
};

void AfeServiceImplTest::SetUpTestCase()
{}

void AfeServiceImplTest::TearDownTestCase()
{}

void AfeServiceImplTest::SetUp()
{}

void AfeServiceImplTest::TearDown()
{}

/*
 * @tc.name: RegisteDataReport_001
 * @tc.desc: register data report.
 * @tc.type: FUNC
 * @tc.require: SR000F5A2Q AR000F8QO2
 * @tc.author: wuzhihui
 */
HWTEST_F(AfeServiceImplTest, RegisteDataReport_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    // 获取afe列表
    afeServiceImpl_.InitSensorServiceImpl();
    std::vector<MedicalSensor> list = afeServiceImpl_.GetSensorList();
    for (size_t i=0; i<list.size(); i++) {
        MedicalSensor afe = list[i];
        printf("get afeiId[%d], afeName[%s], vendorNmae[%s]\n\r", afe.GetSensorId(), 
                afe.GetName().c_str(), afe.GetVendor().c_str());
    }
    ASSERT_NE(list.size(), 0UL);
    // SetBatch
    const int32_t afeId = 0;
    const int32_t afeInterval = 1000000000;
    const int32_t afePollTime = 5;
    int32_t ret = afeServiceImpl_.SetSensorConfig(afeId, afeInterval, afePollTime);
    EXPECT_EQ(0, ret);

    // 注册
    DataCacheFunc cb;
    sptr<ReportDataCache> reportDataCache_;
    reportDataCache_ = new (std::nothrow) ReportDataCache();
    ASSERT_NE(reportDataCache_, nullptr);
    cb = &ReportDataCache::CacheData;
    ret = afeServiceImpl_.RegisteDataReport(cb, reportDataCache_);
    ASSERT_EQ(ret, 0);

    // 激活
    ret = afeServiceImpl_.EnableSensor(afeId);
    EXPECT_EQ(0, ret);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    HiLog::Info(LABEL, "%{public}s begin", "--->Unregister1111");
    // 去注册
    ret = afeServiceImpl_.Unregister();
    ASSERT_EQ(ret, 0);
    HiLog::Info(LABEL, "%{public}s begin", "--->Unregister222");

    // 去激活
    ret = afeServiceImpl_.DisableSensor(afeId);
    ASSERT_EQ(ret, 0);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}
}  // namespace Sensors
}  // namespace OHOS
