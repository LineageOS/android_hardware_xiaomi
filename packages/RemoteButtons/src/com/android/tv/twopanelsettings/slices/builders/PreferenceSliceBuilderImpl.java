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

package com.android.tv.twopanelsettings.slices.builders;

import static android.app.slice.Slice.HINT_ACTIONS;
import static android.app.slice.Slice.HINT_KEYWORDS;
import static android.app.slice.Slice.HINT_LARGE;
import static android.app.slice.Slice.HINT_LAST_UPDATED;
import static android.app.slice.Slice.HINT_LIST_ITEM;
import static android.app.slice.Slice.HINT_NO_TINT;
import static android.app.slice.Slice.HINT_PARTIAL;
import static android.app.slice.Slice.HINT_SHORTCUT;
import static android.app.slice.Slice.HINT_SUMMARY;
import static android.app.slice.Slice.HINT_TITLE;
import static android.app.slice.Slice.HINT_TTL;
import static android.app.slice.Slice.SUBTYPE_CONTENT_DESCRIPTION;
import static android.app.slice.Slice.SUBTYPE_LAYOUT_DIRECTION;
import static android.app.slice.SliceItem.FORMAT_IMAGE;
import static android.app.slice.SliceItem.FORMAT_INT;
import static android.app.slice.SliceItem.FORMAT_TEXT;

import static androidx.slice.builders.ListBuilder.ICON_IMAGE;
import static androidx.slice.builders.ListBuilder.INFINITY;
import static androidx.slice.builders.ListBuilder.LARGE_IMAGE;
import static androidx.slice.core.SliceHints.SUBTYPE_MILLIS;

import static com.android.tv.twopanelsettings.slices.SlicesConstants.BUTTONSTYLE;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_ACTION_ID;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_ADD_INFO_STATUS;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_HAS_END_ICON;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_PAGE_ID;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_PREFERENCE_INFO_IMAGE;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_PREFERENCE_INFO_SUMMARY;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_PREFERENCE_INFO_TEXT;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.EXTRA_PREFERENCE_INFO_TITLE_ICON;
import static com.android.tv.twopanelsettings.slices.SlicesConstants.TYPE_PREFERENCE_EMBEDDED_PLACEHOLDER;

import android.net.Uri;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.graphics.drawable.IconCompat;
import androidx.core.util.Pair;
import androidx.slice.Clock;
import androidx.slice.Slice;
import androidx.slice.SliceItem;
import androidx.slice.SliceSpec;
import androidx.slice.SystemClock;
import androidx.slice.builders.SliceAction;

import com.android.tv.twopanelsettings.slices.builders.PreferenceSliceBuilder.RowBuilder;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;

// TODO: Remove unused code, add test.
/**
 * Provide the implementation for PreferenceSliceBuilder
 */
public class PreferenceSliceBuilderImpl extends TemplateBuilderImpl {

    private List<Slice> mSliceActions;
    public static final String TYPE_PREFERENCE = "TYPE_PREFERENCE";
    public static final String TYPE_PREFERENCE_CATEGORY = "TYPE_PREFERENCE_CATEGORY";
    public static final String TYPE_PREFERENCE_SCREEN_TITLE = "TYPE_PREFERENCE_SCREEN_TITLE";
    public static final String TYPE_FOCUSED_PREFERENCE = "TYPE_FOCUSED_PREFERENCE";
    public static final String TYPE_PREFERENCE_EMBEDDED = "TYPE_PREFERENCE_EMBEDDED";
    public static final String TYPE_PREFERENCE_RADIO = "TYPE_PREFERENCE_RADIO";
    public static final String TYPE_REDIRECTED_SLICE_URI = "TYPE_REDIRECTED_SLICE_URI";
    public static final String TAG_TARGET_URI = "TAG_TARGET_URI";
    public static final String TAG_KEY = "TAG_KEY";
    public static final String TAG_RADIO_GROUP = "TAG_RADIO_GROUP";
    public static final String SUBTYPE_INTENT = "SUBTYPE_INTENT";
    public static final String SUBTYPE_ICON_NEED_TO_BE_PROCESSED =
            "SUBTYPE_ICON_NEED_TO_BE_PROCESSED";
    public static final String SUBTYPE_FOLLOWUP_INTENT = "SUBTYPE_FOLLOWUP_INTENT";
    public static final String SUBTYPE_BUTTON_STYLE = "SUBTYPE_BUTTON_STYLE";
    public static final String SUBTYPE_IS_ENABLED = "SUBTYPE_IS_ENABLED";
    public static final String SUBTYPE_IS_SELECTABLE = "SUBTYPE_IS_SELECTABLE";
    public static final String SUBTYPE_INFO_PREFERENCE = "SUBTYPE_INFO_PREFERENCE";
    public static final String SUBTYPE_SEEKBAR_MIN = "SUBTYPE_SEEKBAR_MIN";
    public static final String SUBTYPE_SEEKBAR_MAX = "SUBTYPE_SEEKBAR_MAX";
    public static final String SUBTYPE_SEEKBAR_VALUE = "SUBTYPE_SEEKBAR_VALUE";

    /**
     *
     */
    public PreferenceSliceBuilderImpl(Slice.Builder b, SliceSpec spec) {
        this(b, spec, new SystemClock());
    }

    /**
     *
     */
    public PreferenceSliceBuilderImpl(Slice.Builder b, SliceSpec spec, Clock clock) {
        super(b, spec, clock);
    }

    /**
     *
     */
    @Override
    public void apply(Slice.Builder builder) {
        builder.addLong(System.currentTimeMillis(), SUBTYPE_MILLIS, HINT_LAST_UPDATED);
        if (mSliceActions != null) {
            Slice.Builder sb = new Slice.Builder(builder);
            for (int i = 0; i < mSliceActions.size(); i++) {
                sb.addSubSlice(mSliceActions.get(i));
            }
            builder.addSubSlice(sb.addHints(HINT_ACTIONS).build());
        }
    }

    /**
     * Add a Preference to the builder
     */
    @NonNull
    public void addPreference(@NonNull RowBuilder builder) {
        addRow(builder, TYPE_PREFERENCE);
    }

    /**
     * Add a PreferenceCategory to the builder
     */
    @NonNull
    public void addPreferenceCategory(@NonNull RowBuilder builder) {
        addRow(builder, TYPE_PREFERENCE_CATEGORY);
    }

    /**
     * Add a screen title to the builder
     */
    @NonNull
    public void addScreenTitle(@NonNull RowBuilder builder) {
        addRow(builder, TYPE_PREFERENCE_SCREEN_TITLE);
    }

    /**
     * Add an embedded preference placeholder where another slice can control the properties via
     * setEmbeddedPreference.
     */
    public void addEmbeddedPreference(@NonNull RowBuilder builder) {
        addRow(builder, TYPE_PREFERENCE_EMBEDDED_PLACEHOLDER);
    }

    /**
     * Set the focused preference for the slice
     */
    @NonNull
    public void setFocusedPreference(CharSequence key) {
        addRow(new RowBuilder().setTitle(key), TYPE_FOCUSED_PREFERENCE);
    }

    /**
     * Set the embedded preference. This preference would be embedded in other TvSettings
     * preference items.
     */
    public void setEmbeddedPreference(@NonNull RowBuilder builder) {
        addRow(builder, TYPE_PREFERENCE_EMBEDDED);
    }

    /**
     * Set the redirected slice uri.
     * @param redirectedSliceUri the new redirected slice uri
     */
    public void setRedirectedSliceUri(CharSequence redirectedSliceUri) {
        addRow(new RowBuilder().setTitle(redirectedSliceUri), TYPE_REDIRECTED_SLICE_URI);
    }

    /**
     * Add a row to list builder.
     */
    @NonNull
    private void addRow(@NonNull RowBuilder builder, String type) {
        RowBuilderImpl impl = new RowBuilderImpl(createChildBuilder());
        impl.fillFrom(builder);
        impl.getBuilder().addHints(HINT_LIST_ITEM);
        getBuilder().addSubSlice(impl.build(), type);
    }

    /**
     *
     */
    private void addAction(@NonNull SliceAction action) {
        if (mSliceActions == null) {
            mSliceActions = new ArrayList<>();
        }
        Slice.Builder b = new Slice.Builder(getBuilder()).addHints(HINT_ACTIONS);
        mSliceActions.add(action.buildSlice(b));
    }

    /**
     *
     */
    private void setKeywords(@NonNull List<String> keywords) {
        Slice.Builder sb = new Slice.Builder(getBuilder());
        for (int i = 0; i < keywords.size(); i++) {
            sb.addText(keywords.get(i), null);
        }
        getBuilder().addSubSlice(sb.addHints(HINT_KEYWORDS).build());
    }

    /**
     *
     */
    public void setNotReady() {
        getBuilder().addHints(HINT_PARTIAL);
    }

    /**
     *
     */
    public void setTtl(long ttl) {
        long expiry = ttl == INFINITY ? INFINITY : System.currentTimeMillis() + ttl;
        getBuilder().addTimestamp(expiry, SUBTYPE_MILLIS, HINT_TTL);
    }

    @RequiresApi(26)
    public void setTtl(@Nullable Duration ttl) {
        setTtl(ttl == null ? INFINITY : ttl.toMillis());
    }

    /**
     *
     */
    public static class RowBuilderImpl extends TemplateBuilderImpl {

        private SliceAction mPrimaryAction;
        private SliceAction mFollowupAction;
        private SliceItem mActionIdItem;
        private SliceItem mPageIdItem;
        private SliceItem mTitleItem;
        private SliceItem mSubtitleItem;
        private Slice mStartItem;
        private ArrayList<Slice> mEndItems = new ArrayList<>();
        private ArrayList<Slice> mInfoItems = new ArrayList<>();
        private CharSequence mContentDescr;
        private SliceItem mUriItem;
        private SliceItem mKeyItem;
        private SliceItem mIconNeedsToBeProcessedItem;
        private SliceItem mButtonStyleItem;
        private SliceItem mSeekbarMinItem;
        private SliceItem mSeekbarMaxItem;
        private SliceItem mSeekbarValueItem;
        private SliceItem mIsEnabledItem;
        private SliceItem mIsSelectableItem;
        private SliceItem mIsAddingInfoStatusItem;
        private SliceItem mRadioGroupItem;
        private SliceItem mInfoTextItem;
        private SliceItem mInfoImageItem;
        private SliceItem mInfoTitleIconItem;
        private SliceItem mInfoSummaryItem;
        private SliceItem mHasEndIconItem;

        /**
         *
         */
        public RowBuilderImpl(@NonNull PreferenceSliceBuilderImpl parent) {
            super(parent.createChildBuilder(), null);
        }

        /**
         *
         */
        public RowBuilderImpl(@NonNull Uri uri) {
            super(new Slice.Builder(uri), null);
        }

        /**
         *
         */
        public RowBuilderImpl(Slice.Builder builder) {
            super(builder, null);
        }

        private void setLayoutDirection(int layoutDirection) {
            getBuilder().addInt(layoutDirection, SUBTYPE_LAYOUT_DIRECTION);
        }

        void fillFrom(RowBuilder builder) {
            if (builder.getUri() != null) {
                setBuilder(new Slice.Builder(builder.getUri()));
            }
            if (builder.getPrimaryAction() != null) {
                setPrimaryAction(builder.getPrimaryAction());
            }
            if (builder.getFollowupAction() != null) {
                setFollowupAction(builder.getFollowupAction());
            }
            if (builder.getLayoutDirection() != -1) {
                setLayoutDirection(builder.getLayoutDirection());
            }
            setActionId(builder.getActionId());
            setPageId(builder.getPageId());
            if (builder.getTitleAction() != null || builder.isTitleActionLoading()) {
                setTitleItem(builder.getTitleAction(), builder.isTitleActionLoading());
            } else if (builder.getTitleIcon() != null || builder.isTitleItemLoading()) {
                setTitleItem(builder.getTitleIcon(), 0,
                        builder.isTitleItemLoading());
            }
            if (builder.getTitle() != null || builder.isTitleLoading()) {
                setTitle(builder.getTitle(), builder.isTitleLoading());
            }
            if (builder.getSubtitle() != null || builder.isSubtitleLoading()) {
                setSubtitle(builder.getSubtitle(), builder.isSubtitleLoading());
            }
            if (builder.getContentDescription() != null) {
                setContentDescription(builder.getContentDescription());
            }
            if (builder.getTargetSliceUri() != null) {
                setTargetSliceUri(builder.getTargetSliceUri());
            }
            if (builder.getKey() != null) {
                setKey(builder.getKey());
            }
            if (builder.getTitleIcon() != null) {
                setIconNeedsToBeProcessed(builder.iconNeedsToBeProcessed());
            }
            if (builder.getRadioGroup() != null) {
                setRadioGroup(builder.getRadioGroup());
            }
            if (builder.getInfoText() != null) {
                setInfoText(builder.getInfoText());
            }
            if (builder.getInfoTitle() != null) {
                setInfoTitle(builder.getInfoTitle());
            }
            if (builder.getInfoSummary() != null) {
                setInfoSummary(builder.getInfoSummary());
            }
            if (builder.getInfoTitleIcon() != null) {
                setInfoTitleIcon(builder.getInfoTitleIcon());
            }
            if (builder.getInfoImage() != null) {
                setInfoImage(builder.getInfoImage());
            }
            setHasEndIcon(builder.hasEndIcon());
            setButtonStyle(builder.getButtonStyle());
            setSeekbarMin(builder.getSeekbarMin());
            setSeekbarMax(builder.getSeekbarMax());
            setSeekbarValue(builder.getSeekbarValue());
            setEnabled(builder.isEnabled());
            setSelectable(builder.isSelectable());
            setAddInfoStatus(builder.addInfoStatus());
            List<Object> endItems = builder.getEndItems();
            List<Integer> endTypes = builder.getEndTypes();
            List<Boolean> endLoads = builder.getEndLoads();
            for (int i = 0; i < endItems.size(); i++) {
                switch (endTypes.get(i)) {
                    case RowBuilder.TYPE_ACTION:
                        addEndItem((SliceAction) endItems.get(i), endLoads.get(i));
                        break;
                    case RowBuilder.TYPE_ICON:
                        Pair<IconCompat, Integer> pair =
                                (Pair<IconCompat, Integer>) endItems.get(i);
                        addEndItem(pair.first, pair.second, endLoads.get(i));
                        break;
                }
            }

            List<Pair<String, String>> infoItems = builder.getInfoItems();
            for (int i = 0; i < infoItems.size(); i++) {
                addInfoItem(infoItems.get(i).first, infoItems.get(i).second);
            }
        }

        /**
         *
         */
        @NonNull
        private void setTitleItem(IconCompat icon, int imageMode) {
            setTitleItem(icon, imageMode, false /* isLoading */);
        }

        private void addInfoItem(String title, String summary) {
            Slice.Builder sb = new Slice.Builder(getBuilder())
                        .addText(title, null, HINT_TITLE)
                        .addText(summary, null, HINT_SUMMARY);
            mInfoItems.add(sb.build());
        }

        /**
         *
         */
        @NonNull
        private void setTitleItem(IconCompat icon, int imageMode, boolean isLoading) {
            ArrayList<String> hints = new ArrayList<>();
            if (imageMode != ICON_IMAGE) {
                hints.add(HINT_NO_TINT);
            }
            if (imageMode == LARGE_IMAGE) {
                hints.add(HINT_LARGE);
            }
            if (isLoading) {
                hints.add(HINT_PARTIAL);
            }
            Slice.Builder sb = new Slice.Builder(getBuilder())
                    .addIcon(icon, null /* subType */, hints);
            if (isLoading) {
                sb.addHints(HINT_PARTIAL);
            }
            mStartItem = sb.addHints(HINT_TITLE).build();
        }

        /**
         *
         */
        @NonNull
        private void setTitleItem(@NonNull SliceAction action) {
            setTitleItem(action, false /* isLoading */);
        }

        /**
         *
         */
        private void setTitleItem(SliceAction action, boolean isLoading) {
            Slice.Builder sb = new Slice.Builder(getBuilder()).addHints(HINT_TITLE);
            if (isLoading) {
                sb.addHints(HINT_PARTIAL);
            }
            mStartItem = action.buildSlice(sb);
        }

        /**
         *
         */
        @NonNull
        public void setPrimaryAction(@NonNull SliceAction action) {
            mPrimaryAction = action;
        }

        public void setFollowupAction(@NonNull SliceAction action) {
            mFollowupAction = action;
        }

        /** Set the actionId to be digested for logging. */
        public void setActionId(int actionId) {
            mActionIdItem = new SliceItem(actionId, FORMAT_INT, EXTRA_ACTION_ID, new String[]{});
        }

        /** Set the pageId to be digested for logging. */
        public void setPageId(int pageId) {
            mPageIdItem = new SliceItem(pageId, FORMAT_INT, EXTRA_PAGE_ID, new String[]{});
        }

        /**
         *
         */
        @NonNull
        public void setTitle(CharSequence title) {
            setTitle(title, false /* isLoading */);
        }

        /**
         *
         */
        public void setTitle(CharSequence title, boolean isLoading) {
            mTitleItem = new SliceItem(title, FORMAT_TEXT, null, new String[]{HINT_TITLE});
            if (isLoading) {
                mTitleItem.addHint(HINT_PARTIAL);
            }
        }

        public void setTargetSliceUri(CharSequence uri) {
            mUriItem = new SliceItem(uri, FORMAT_TEXT, TAG_TARGET_URI, new String[]{HINT_ACTIONS});
        }

        /**
         *
         */
        public void setIconNeedsToBeProcessed(boolean needed) {
            mIconNeedsToBeProcessedItem = new SliceItem(
                    needed ? 1 : 0, FORMAT_INT, SUBTYPE_ICON_NEED_TO_BE_PROCESSED, new String[]{});
        }

        public void setHasEndIcon(boolean hasEndIcon) {
            mHasEndIconItem = new SliceItem(
                    hasEndIcon ? 1 : 0, FORMAT_INT, EXTRA_HAS_END_ICON, new String[]{});
        }

        /**
         *
         */
        public void setButtonStyle(@BUTTONSTYLE int buttonStyle) {
            mButtonStyleItem = new SliceItem(
                    buttonStyle, FORMAT_INT, SUBTYPE_BUTTON_STYLE, new String[]{});
        }

        /**
         * Set minimum value of the seekbar.
         */
        public void setSeekbarMin(int value) {
            mSeekbarMinItem = new SliceItem(
                    value, FORMAT_INT, SUBTYPE_SEEKBAR_MIN, new String[]{});
        }

        /**
         * Set maximum value of the seekbar.
         */
        public void setSeekbarMax(int value) {
            mSeekbarMaxItem = new SliceItem(
                    value, FORMAT_INT, SUBTYPE_SEEKBAR_MAX, new String[]{});
        }

        /**
         * Set progress value of the seekbar.
         */
        public void setSeekbarValue(int value) {
            mSeekbarValueItem = new SliceItem(
                    value, FORMAT_INT, SUBTYPE_SEEKBAR_VALUE, new String[]{});
        }

        /**
         *
         */
        public void setRadioGroup(CharSequence radioGroup) {
            mRadioGroupItem = new SliceItem(
                    radioGroup, FORMAT_TEXT, TAG_RADIO_GROUP, new String[]{});
        }

        /**
         *
         */
        public void setEnabled(boolean enabled) {
            mIsEnabledItem = new SliceItem(
                    enabled ? 1 : 0, FORMAT_INT, SUBTYPE_IS_ENABLED, new String[]{});
        }

        public void setSelectable(boolean selectable) {
            mIsSelectableItem = new SliceItem(
                    selectable ? 1 : 0, FORMAT_INT, SUBTYPE_IS_SELECTABLE, new String[]{});
        }

        public void setAddInfoStatus(boolean addInfoStatus) {
            mIsAddingInfoStatusItem = new SliceItem(
                    addInfoStatus ? 1 : 0, FORMAT_INT, EXTRA_ADD_INFO_STATUS, new String[]{});
        }

        public void setKey(CharSequence key) {
            mKeyItem = new SliceItem(key, FORMAT_TEXT, TAG_KEY, new String[] {HINT_KEYWORDS});
        }

        public void setInfoText(CharSequence infoText) {
            setInfoTitle(infoText);
        }

        public void setInfoImage(IconCompat infoImage) {
            mInfoImageItem = new SliceItem(
                    infoImage, FORMAT_IMAGE, EXTRA_PREFERENCE_INFO_IMAGE, new String[]{});
        }

        public void setInfoTitleIcon(IconCompat infoImage) {
            mInfoTitleIconItem = new SliceItem(
                    infoImage, FORMAT_IMAGE, EXTRA_PREFERENCE_INFO_TITLE_ICON, new String[]{});
        }

        public void setInfoTitle(CharSequence infoTitle) {
            mInfoTextItem = new SliceItem(
                    infoTitle, FORMAT_TEXT, EXTRA_PREFERENCE_INFO_TEXT, new String[]{});
        }

        public void setInfoSummary(CharSequence infoSummary) {
            mInfoSummaryItem = new SliceItem(
                    infoSummary, FORMAT_TEXT, EXTRA_PREFERENCE_INFO_SUMMARY, new String[]{});
        }

        /**
         *
         */
        @NonNull
        public void setSubtitle(CharSequence subtitle) {
            setSubtitle(subtitle, false /* isLoading */);
        }

        /**
         *
         */
        public void setSubtitle(CharSequence subtitle, boolean isLoading) {
            mSubtitleItem = new SliceItem(subtitle, FORMAT_TEXT, null, new String[0]);
            if (isLoading) {
                mSubtitleItem.addHint(HINT_PARTIAL);
            }
        }

        /**
         *
         */
        @NonNull
        public void addEndItem(IconCompat icon, int imageMode) {
            addEndItem(icon, imageMode, false /* isLoading */);
        }

        /**
         *
         */
        @NonNull
        public void addEndItem(IconCompat icon, int imageMode, boolean isLoading) {
            ArrayList<String> hints = new ArrayList<>();
            if (imageMode != ICON_IMAGE) {
                hints.add(HINT_NO_TINT);
            }
            if (imageMode == LARGE_IMAGE) {
                hints.add(HINT_LARGE);
            }
            if (isLoading) {
                hints.add(HINT_PARTIAL);
            }
            Slice.Builder sb = new Slice.Builder(getBuilder())
                    .addIcon(icon, null /* subType */, hints);
            if (isLoading) {
                sb.addHints(HINT_PARTIAL);
            }
            mEndItems.add(sb.build());
        }

        /**
         *
         */
        @NonNull
        public void addEndItem(@NonNull SliceAction action) {
            addEndItem(action, false /* isLoading */);
        }

        /**
         *
         */
        public void addEndItem(@NonNull SliceAction action, boolean isLoading) {
            Slice.Builder sb = new Slice.Builder(getBuilder());
            if (isLoading) {
                sb.addHints(HINT_PARTIAL);
            }
            mEndItems.add(action.buildSlice(sb));
        }

        public void setContentDescription(CharSequence description) {
            mContentDescr = description;
        }

        /**
         *
         */
        @Override
        public void apply(Slice.Builder b) {
            if (mStartItem != null) {
                b.addSubSlice(mStartItem);
            }
            if (mTitleItem != null) {
                b.addItem(mTitleItem);
            }
            if (mSubtitleItem != null) {
                b.addItem(mSubtitleItem);
            }
            if (mUriItem != null) {
                b.addItem(mUriItem);
            }
            if (mKeyItem != null) {
                b.addItem(mKeyItem);
            }
            if (mIconNeedsToBeProcessedItem != null) {
                b.addItem(mIconNeedsToBeProcessedItem);
            }
            if (mButtonStyleItem != null) {
                b.addItem(mButtonStyleItem);
            }
            if (mIsEnabledItem != null) {
                b.addItem(mIsEnabledItem);
            }
            if (mIsSelectableItem != null) {
                b.addItem(mIsSelectableItem);
            }
            if (mRadioGroupItem != null) {
                b.addItem(mRadioGroupItem);
            }
            if (mInfoTextItem != null) {
                b.addItem(mInfoTextItem);
            }
            if (mInfoImageItem != null) {
                b.addItem(mInfoImageItem);
            }
            if (mActionIdItem != null) {
                b.addItem(mActionIdItem);
            }
            if (mPageIdItem != null) {
                b.addItem(mPageIdItem);
            }
            if (mInfoTitleIconItem != null) {
                b.addItem(mInfoTitleIconItem);
            }
            if (mInfoSummaryItem != null) {
                b.addItem(mInfoSummaryItem);
            }
            if (mIsAddingInfoStatusItem != null) {
                b.addItem(mIsAddingInfoStatusItem);
            }
            if (mSeekbarMinItem != null) {
                b.addItem(mSeekbarMinItem);
            }
            if (mSeekbarMaxItem != null) {
                b.addItem(mSeekbarMaxItem);
            }
            if (mSeekbarValueItem != null) {
                b.addItem(mSeekbarValueItem);
            }
            if (mHasEndIconItem != null) {
                b.addItem(mHasEndIconItem);
            }
            for (int i = 0; i < mEndItems.size(); i++) {
                Slice item = mEndItems.get(i);
                b.addSubSlice(item);
            }
            for (int i = 0; i < mInfoItems.size(); i++) {
                Slice item = mInfoItems.get(i);
                b.addSubSlice(item, SUBTYPE_INFO_PREFERENCE);
            }
            if (mContentDescr != null) {
                b.addText(mContentDescr, SUBTYPE_CONTENT_DESCRIPTION);
            }
            if (mPrimaryAction != null) {
                Slice.Builder sb = new Slice.Builder(
                        getBuilder()).addHints(HINT_TITLE, HINT_SHORTCUT);
                b.addSubSlice(mPrimaryAction.buildSlice(sb), SUBTYPE_INTENT);
            }
            if (mFollowupAction != null) {
                Slice.Builder sb = new Slice.Builder(
                        getBuilder()).addHints(HINT_TITLE, HINT_SHORTCUT);
                b.addSubSlice(mFollowupAction.buildSlice(sb), SUBTYPE_FOLLOWUP_INTENT);
            }
        }
    }
}
