/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_
#define CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_

#include <stdbool.h>

#include "mgos_updater_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if MGOS_ENABLE_UPDATER
void mgos_ota_http_start(struct update_context *ctx, const char *url);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_ */
