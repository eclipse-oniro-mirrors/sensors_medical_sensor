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
#include <memory>
#include <thread>
#include <unistd.h>

#include <fcntl.h>

#include <gtest/gtest.h>
#include "medical_data_channel.h"
#include "medical_service_client.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "string_ex.h"


namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_TEST, "AfeDFXTest" };
const std::string CMD_LINE = "ps -ef | grep 'hhealth' | grep -v grep | awk '{print $2}'";
constexpr int32_t BUFFER_SIZE = 8;
constexpr pid_t INVALID_PID = -1;
}  // namespace

class AfeDFXTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    pid_t GetSensorServicePid();
    static void HandleEvent(struct SensorEvent *events, int32_t num, void *data);
    static std::vector<MedicalSensor> g_afesList;
    static sptr<MedicalSensorDataChannel> g_afeDataChannel;
    static std::unique_ptr<MedicalSensorServiceClient> g_afeServiceClient;
    static bool g_dataReport;
};

std::vector<MedicalSensor> AfeDFXTest::g_afesList;
sptr<MedicalSensorDataChannel> AfeDFXTest::g_afeDataChannel = nullptr;
bool AfeDFXTest::g_dataReport = false;
std::unique_ptr<MedicalSensorServiceClient> AfeDFXTest::g_afeServiceClient = nullptr;

pid_t AfeDFXTest::GetSensorServicePid()
{
    pid_t pid = INVALID_PID;
    char buf[BUFFER_SIZE] = { 0 };
    FILE *fp = popen(CMD_LINE.c_str(), "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, "get error when getting afe service process id");
        return pid;
    }

    fgets(buf, sizeof(buf) - 1, fp);
    pclose(fp);
    fp = nullptr;
    HiLog::Info(LABEL, "process is : %{public}s", buf);

    std::string pidStr(buf);
    pidStr = TrimStr(pidStr, '\n');
    HiLog::Info(LABEL, "pidStr is : %{public}s", pidStr.c_str());
    if (pidStr.empty()) {
        return pid;
    }

    if (IsNumericStr(pidStr)) {
        pid = std::stoi(pidStr);
    }
    return pid;
}

void AfeDFXTest::HandleEvent(struct SensorEvent *events, int32_t num, void *data)
{
    HiLog::Info(LABEL, "%{public}s HandleEvent", __func__);
    for (int32_t i = 0; i < num; i++) {
        g_dataReport = true;
    }
    return;
}

void AfeDFXTest::SetUpTestCase()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    g_afeDataChannel = new (std::nothrow) MedicalSensorDataChannel();
    ASSERT_NE(g_afeDataChannel, nullptr);
    g_afeServiceClient = std::make_unique<MedicalSensorServiceClient>();
    ASSERT_NE(g_afeServiceClient, nullptr);
    g_afesList = g_afeServiceClient->GetSensorList();
    ASSERT_NE(g_afesList.size(), 0UL);
    auto ret = g_afeDataChannel->CreateSensorDataChannel(HandleEvent, nullptr);
    HiLog::Info(LABEL, "CreateSensorDataChannel ret is : %{public}d", ret);
    ASSERT_EQ(ret, ERR_OK);
    ret = g_afeServiceClient->TransferDataChannel(g_afeDataChannel);
    HiLog::Info(LABEL, "TransferDataChannel ret is : %{public}d", ret);
    ASSERT_EQ(ret, ERR_OK);
    g_dataReport = false;
}

void AfeDFXTest::TearDownTestCase()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    g_afeServiceClient->DestroyDataChannel();
    g_afeDataChannel->DestroySensorDataChannel();
}

void AfeDFXTest::SetUp()
{}

void AfeDFXTest::TearDown()
{}
}  // namespace Sensors
}  // namespace OHOS
