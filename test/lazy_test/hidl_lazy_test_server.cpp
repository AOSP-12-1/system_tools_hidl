/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/logging.h>
#include <android/hardware/tests/lazy/1.1/ILazy.h>
#include <hidl/HidlLazyUtils.h>
#include <hidl/HidlTransportSupport.h>

using android::OK;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::LazyServiceRegistrar;
using android::hardware::tests::lazy::V1_1::ILazy;

class Lazy : public ILazy {
  public:
    ::android::hardware::Return<void> setCustomActiveServicesCallback();
};

::android::hardware::Return<void> Lazy::setCustomActiveServicesCallback() {
    auto lazyRegistrar = android::hardware::LazyServiceRegistrar::getInstance();
    lazyRegistrar.setActiveServicesCallback([lazyRegistrar](bool hasClients) mutable -> bool {
        if (hasClients) {
            return false;
        }

        // Unregister all services
        if (!lazyRegistrar.tryUnregister()) {
            // Prevent shutdown (test will fail)
            return true;
        }

        // Re-register all services
        lazyRegistrar.reRegister();

        // Unregister again before shutdown
        if (!lazyRegistrar.tryUnregister()) {
            // Prevent shutdown (test will fail)
            return true;
        }

        exit(EXIT_SUCCESS);
        // Unreachable
    });

    return ::android::hardware::Status::ok();
}

int main() {
    configureRpcThreadpool(1, true /*willJoin*/);
    CHECK(OK == LazyServiceRegistrar::getInstance().registerService(new Lazy, "default1"));
    CHECK(OK == LazyServiceRegistrar::getInstance().registerService(new Lazy, "default2"));
    joinRpcThreadpool();
    return EXIT_FAILURE;  // should not reach
}
