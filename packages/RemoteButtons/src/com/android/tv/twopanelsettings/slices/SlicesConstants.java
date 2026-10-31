/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.android.tv.twopanelsettings.slices;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Constants for slice.
 */
public final class SlicesConstants {
    public static final String TYPE_PREFERENCE = "TYPE_PREFERENCE";
    public static final String TYPE_PREFERENCE_CATEGORY = "TYPE_PREFERENCE_CATEGORY";
    public static final String TYPE_PREFERENCE_SCREEN_TITLE = "TYPE_PREFERENCE_SCREEN_TITLE";
    public static final String TYPE_PREFERENCE_EMBEDDED = "TYPE_PREFERENCE_EMBEDDED";
    public static final String TYPE_PREFERENCE_EMBEDDED_PLACEHOLDER =
            "TYPE_PREFERENCE_EMBEDDED_PLACEHOLDER";
    public static final String TYPE_FOCUSED_PREFERENCE = "TYPE_FOCUSED_PREFERENCE";
    public static final String TYPE_REDIRECTED_SLICE_URI = "TYPE_REDIRECTED_SLICE_URI";
    public static final String TAG_TARGET_URI = "TAG_TARGET_URI";
    public static final String TAG_SCREEN_TITLE = "TAG_SCREEN_TITLE";
    public static final String TAG_KEY = "TAG_KEY";
    public static final String TAG_RADIO_GROUP = "TAG_RADIO_GROUP";
    public static final String SUBTYPE_INTENT = "SUBTYPE_INTENT";
    public static final String SUBTYPE_FOLLOWUP_INTENT = "SUBTYPE_FOLLOWUP_INTENT";
    public static final String SUBTYPE_ICON_NEED_TO_BE_PROCESSED =
            "SUBTYPE_ICON_NEED_TO_BE_PROCESSED";
    public static final String SUBTYPE_BUTTON_STYLE = "SUBTYPE_BUTTON_STYLE";
    public static final String SUBTYPE_IS_ENABLED = "SUBTYPE_IS_ENABLED";
    public static final String SUBTYPE_IS_SELECTABLE = "SUBTYPE_IS_SELECTABLE";
    public static final String SUBTYPE_INFO_PREFERENCE = "SUBTYPE_INFO_PREFERENCE";
    public static final String SUBTYPE_SEEKBAR_MIN = "SUBTYPE_SEEKBAR_MIN";
    public static final String SUBTYPE_SEEKBAR_MAX = "SUBTYPE_SEEKBAR_MAX";
    public static final String SUBTYPE_SEEKBAR_VALUE = "SUBTYPE_SEEKBAR_VALUE";
    public static final String PATH_STATUS = "status";
    public static final String PARAMETER_DIRECTION = "direction";
    public static final String FORWARD = "forward";
    public static final String BACKWARD = "back";
    public static final String EXIT = "exit";
    public static final String PARAMETER_ERROR = "error";
    public static final String PARAMETER_URI = "uri";
    public static final String EXTRA_PREFERENCE_KEY = "extra_preference_key";
    public static final String EXTRA_PREFERENCE_INFO_TEXT= "extra_preference_info_text";
    public static final String EXTRA_PREFERENCE_INFO_IMAGE = "extra_preference_info_image";
    public static final String EXTRA_PREFERENCE_INFO_TITLE_ICON =
            "extra_preference_info_title_icon";
    public static final String EXTRA_PREFERENCE_INFO_SUMMARY = "extra_preference_info_summary";
    public static final String EXTRA_PREFERENCE_INFO_STATUS = "extra_preference_info_status";
    public static final String EXTRA_ADD_INFO_STATUS = "extra_preference_add_info_status";
    public static final String EXTRA_ACTION_ID = "extra_action_id";
    public static final String EXTRA_PAGE_ID = "extra_page_id";
    public static final String EXTRA_SLICE_FOLLOWUP = "extra_slice_followup";
    public static final String EXTRA_HAS_END_ICON = "extra_has_end_icon";
    public static final int SWITCH = 0;
    public static final int CHECKMARK = 1;
    public static final int RADIO = 2;
    public static final int SEEKBAR = 3;

    @IntDef({SWITCH, CHECKMARK, RADIO, SEEKBAR})
    @Retention(RetentionPolicy.SOURCE)
    public @interface BUTTONSTYLE {}
}
