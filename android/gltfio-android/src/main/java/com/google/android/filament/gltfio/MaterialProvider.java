/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.google.android.filament.gltfio;

import com.google.android.filament.MaterialInstance;
import com.google.android.filament.Material;
import com.google.android.filament.VertexBuffer;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

public interface MaterialProvider {

    public enum AlphaMode {
        OPAQUE,
        MASK,
        BLEND
    };

    /**
     * MaterialKey specifies the requirements for a requested glTF material.
     * The provider creates Filament materials that fulfill these requirements.
     */
    public class MaterialKey {
        boolean doubleSided;
        boolean unlit;
        boolean hasVertexColors;
        boolean hasBaseColorTexture;
        boolean hasNormalTexture;
        boolean hasOcclusionTexture;
        boolean hasEmissiveTexture;
        boolean useSpecularGlossiness;
        AlphaMode alphaMode;
        boolean enableDiagnostics;
        boolean hasMetallicRoughnessTexture; // piggybacks with specularRoughness
        int metallicRoughnessUV;             // piggybacks with specularRoughness
        int baseColorUV;
        boolean hasClearCoatTexture;
        int clearCoatUV;
        boolean hasClearCoatRoughnessTexture;
        int clearCoatRoughnessUV;
        boolean hasClearCoatNormalTexture;
        int clearCoatNormalUV;
        boolean hasClearCoat;
        boolean hasTransmission;
        boolean hasTextureTransforms;
        int emissiveUV;
        int aoUV;
        int normalUV;
        boolean hasTransmissionTexture;
        int transmissionUV;
        boolean hasSheenColorTexture;
        int sheenColorUV;
        boolean hasSheenRoughnessTexture;
        int sheenRoughnessUV;
        boolean hasSheen;
        boolean hasIOR;
    };

    public enum UvSet {
        UNUSED, UV0, UV1;
        public static final UvSet values[] = values();
    };

    /**
     * Creates or fetches a compiled Filament material, then creates an instance from it.
     *
     * @param config Specifies requirements; might be mutated due to resource constraints.
     * @param uvmap Output argument that gets populated with a small table that maps from a glTF uv
     *              index to a Filament uv index.
     * @param label Optional tag that is not a part of the cache key.
     */
    public @Nullable MaterialInstance createMaterialInstance(MaterialKey config,
            @NonNull @Size(min = 8) UvSet[] uvmap, @Nullable String label);

    /**
     * Creates and returns an array containing all cached materials.
     */
    public @NonNull Material[] getMaterials();

    /**
     * Returns true if the presence of the given vertex attribute is required.
     *
     * Some types of providers (e.g. ubershader) require dummy attribute values
     * if the glTF model does not provide them.
     */
    public boolean needsDummyData(VertexBuffer.VertexAttribute attrib);

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    public void destroyMaterials();
}
