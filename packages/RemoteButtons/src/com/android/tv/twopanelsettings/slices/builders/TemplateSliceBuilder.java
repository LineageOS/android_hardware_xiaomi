/*
 * Copyright 2019 The Android Open Source Project
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

import android.content.Context;
import android.net.Uri;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.slice.Clock;
import androidx.slice.Slice;
import androidx.slice.SliceManager;
import androidx.slice.SliceProvider;
import androidx.slice.SliceSpec;
import androidx.slice.SystemClock;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * A copy of TemplateSliceBuilder from slices support lib. Base class of builders of various
 * template types.
 */
public abstract class TemplateSliceBuilder {

    private static final String TAG = "TemplateSliceBuilder";

    private final Slice.Builder mBuilder;
    private final Context mContext;
    private final TemplateBuilderImpl mImpl;
    private List<SliceSpec> mSpecs;

    protected TemplateSliceBuilder(TemplateBuilderImpl impl) {
        mContext = null;
        mBuilder = null;
        mImpl = impl;
        setImpl(impl);
    }

    public TemplateSliceBuilder(Context context, Uri uri) {
        mBuilder = new Slice.Builder(uri);
        mContext = context;
        mSpecs = getSpecs(uri);
        mImpl = selectImpl(uri);
        if (mImpl == null) {
            throw new IllegalArgumentException("No valid specs found");
        }
        setImpl(mImpl);
    }

    /**
     * Construct the slice.
     */
    @NonNull
    public Slice build() {
        return mImpl.build();
    }

    protected Slice.Builder getBuilder() {
        return mBuilder;
    }

    abstract void setImpl(TemplateBuilderImpl impl);

    protected TemplateBuilderImpl selectImpl(Uri uri) {
        return null;
    }

    protected boolean checkCompatible(SliceSpec candidate, Uri uri) {
        final int size = mSpecs.size();
        for (int i = 0; i < size; i++) {
            if (mSpecs.get(i).canRender(candidate)) {
                return true;
            }
        }
        return false;
    }

    private List<SliceSpec> getSpecs(Uri uri) {
        if (SliceProvider.getCurrentSpecs() != null) {
            return new ArrayList<>(SliceProvider.getCurrentSpecs());
        }
        Set<SliceSpec> pinnedSpecs = SliceManager.getInstance(mContext).getPinnedSpecs(uri);
        return new ArrayList<>(pinnedSpecs);
    }

    protected Clock getClock() {
        if (SliceProvider.getClock() != null) {
            return SliceProvider.getClock();
        }
        return new SystemClock();
    }

    /**
     * This is for typing, to clean up the code.
     *
     * @hide
     */
    static <T> Pair<SliceSpec, Class<? extends TemplateBuilderImpl>> pair(SliceSpec spec,
            Class<T> cls) {
        return new Pair(spec, cls);
    }
}
