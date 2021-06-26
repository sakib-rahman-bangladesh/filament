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

#include <jni.h>

#include <filament/Engine.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <utils/debug.h>

#include "common/NioUtils.h"

using namespace filament;
using namespace gltfio;
using namespace utils;

class JavaMaterialProvider : public MaterialProvider {
    JNIEnv* const mEnv;
    const jobject mJavaProvider;
    const Material* const*  mPreviousMaterials = nullptr;

    jclass mMaterialKey;
    jmethodID mCreateMaterialInstance;
    jmethodID mGetMaterials;
    jmethodID mNeedsDummyData;
    jmethodID mDestroyMaterials;

public:
    JavaMaterialProvider(JNIEnv* env, jobject provider) : mEnv(env), mJavaProvider(provider) {
        mMaterialKey = env->FindClass(
                "com/google/android/filament/gltfio/MaterialProvider$MaterialKey");
        mMaterialKey = (jclass) env->NewGlobalRef(mMaterialKey);
        assert_invariant(mMaterialKey);

        jclass providerClass = env->GetObjectClass(provider);

        mCreateMaterialInstance = env->GetMethodID(providerClass, "createMaterialInstance",
                "(Lcom/google/android/filament/gltfio/MaterialProvider$MaterialKey;"
                "[Lcom/google/android/filament/gltfio/MaterialProvider$UvSet;"
                "Ljava/lang/String;)"
                "Lcom/google/android/filament/MaterialInstance;");
        assert_invariant(mCreateMaterialInstance);

        mGetMaterials = env->GetMethodID(providerClass, "getMaterials",
                "()[Lcom/google/android/filament/Material;");
        assert_invariant(mGetMaterials()

        mNeedsDummyData = env->GetMethodID(providerClass, "needsDummyData",
                "(Lcom/google/android/filament/VertexBuffer$VertexAttribute;)Z");
        assert_invariant(mNeedsDummyData()

        mDestroyMaterials = env->GetMethodID(providerClass, "destroyMaterials", "()V");
        assert_invariant(mDestroyMaterials()
    }

    ~JavaMaterialProvider() override {
        mEnv->DeleteGlobalRef(mMaterialKey);
        delete mPreviousMaterials;
    }

    MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label) override {

        // TODO: 1. create a Java object for "MaterialKey"
        //       2. call the method
        //       3. modify the C++ object for "MaterialKey"
        //       4. release the Java object for "MaterialKey" (I think)

        return nullptr;
    }

    const Material* const* getMaterials() const noexcept override {
        delete mPreviousMaterials;
        mPreviousMaterials = new (const Material* const)[5];
        // TODO: Copy over materials from the Java object
        return mPreviousMaterials;
    }

    size_t getMaterialsCount() const noexcept override {
        // TODO: getMaterialsCount
        return 0;
    }

    void destroyMaterials() override {
        // TODO: destroyMaterials
    }

    bool needsDummyData(VertexAttribute attrib) const noexcept override {
        return false;
    }
};

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetLoader(JNIEnv* env, jclass,
        jlong nativeEngine, jobject provider, jlong nativeEntities) {
    Engine* engine = (Engine*) nativeEngine;
    MaterialProvider* materials = new JavaMaterialProvider(env, provider);
    EntityManager* entities = (EntityManager*) nativeEntities;
    NameComponentManager* names = new NameComponentManager(*entities);
    return (jlong) AssetLoader::create({engine, materials, names, entities});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAssetLoader(JNIEnv*, jclass,
        jlong nativeLoader) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    delete loader->getMaterialProvider();
    NameComponentManager* names = loader->getNames();
    AssetLoader::destroy(&loader);
    delete names;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetFromBinary(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    return (jlong) loader->createAssetFromBinary((const uint8_t *) buffer.getData(),
            buffer.getSize());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetFromJson(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    return (jlong) loader->createAssetFromJson((const uint8_t *) buffer.getData(),
            buffer.getSize());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateInstancedAsset(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining, jlongArray instances) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    jsize numInstances = env->GetArrayLength(instances);
    using Handle = FilamentInstance*;
    Handle* ptrInstances = new Handle[numInstances];
    jlong asset = (jlong) loader->createInstancedAsset((const uint8_t *) buffer.getData(),
            buffer.getSize(), ptrInstances, numInstances);
    if (asset) {
        jlong* longInstances = env->GetLongArrayElements(instances, nullptr);
        for (jsize i = 0; i < numInstances; i++) {
            longInstances[i] = (jlong) ptrInstances[i];
        }
        env->ReleaseLongArrayElements(instances, longInstances, 0);
    }
    delete[] ptrInstances;
    return asset;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nEnableDiagnostics(JNIEnv*, jclass,
        jlong nativeLoader, jboolean enable) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    loader->enableDiagnostics(enable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAsset(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    loader->destroyAsset(asset);
}
