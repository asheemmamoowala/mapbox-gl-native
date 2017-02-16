#include "file_source.hpp"

#include <mbgl/util/logging.hpp>

#include <string>

#include "jni/generic_global_ref_deleter.hpp"


namespace mbgl {
namespace android {

jni::Class<FileSource::ResourceTransformCallback> FileSource::ResourceTransformCallback::javaClass;

std::string FileSource::ResourceTransformCallback::onURL(jni::JNIEnv& env, jni::Object<FileSource::ResourceTransformCallback> callback, int kind, std::string url_) {
    static auto method = FileSource::ResourceTransformCallback::javaClass.GetMethod<jni::String (jni::jint, jni::String)>(env, "onURL");
    auto url = jni::Make<jni::String>(env, url_);
    url = callback.Call(env, method, kind, url);
    return jni::Make<std::string>(env, url);
}

FileSource::FileSource(jni::JNIEnv& _env, jni::String _cachePath, jni::String _apkPath) {
    Log::Info(Event::JNI, "FileSource::FileSource");

    // Create a core default file source
    fileSource = std::make_unique<mbgl::DefaultFileSource>(
        jni::Make<std::string>(_env, _cachePath) + "/mbgl-offline.db",
        jni::Make<std::string>(_env, _apkPath));
}

FileSource::~FileSource() {
    Log::Info(Event::JNI, "FileSource::~FileSource");
}

void FileSource::setResourceTransform(jni::JNIEnv& env, jni::Object<FileSource::ResourceTransformCallback> transformCallback) {
    if (transformCallback) {
        // Launch transformCallback
        fileSource->setResourceTransform([
            // Capture the ResourceTransformCallback object as a managed global into
            // the lambda. It is released automatically when we're setting a new ResourceTransform in
            // a subsequent call.
            // Note: we're converting it to shared_ptr because this lambda is converted to a std::function,
            // which requires copyability of its captured variables.
            callback = std::shared_ptr<jni::jobject>(transformCallback.NewGlobalRef(env).release()->Get(), GenericGlobalRefDeleter()),
            env
        ](mbgl::Resource::Kind kind, std::string&& url_) {
            return FileSource::ResourceTransformCallback::onURL(const_cast<jni::JNIEnv&>(env), jni::Object<FileSource::ResourceTransformCallback>(*callback), int(kind), url_);
        });
    } else {
        // Reset the callback
        fileSource->setResourceTransform(nullptr);
    }
}

jni::Class<FileSource> FileSource::javaClass;

FileSource* FileSource::getNativePeer(jni::JNIEnv& env, jni::Object<FileSource> jFileSource) {
    static auto field = FileSource::javaClass.GetField<jlong>(env, "nativePtr");
    return reinterpret_cast<FileSource *>(jFileSource.Get(env, field));
}

mbgl::DefaultFileSource& FileSource::getDefaultFileSource(jni::JNIEnv& env, jni::Object<FileSource> jFileSource) {
    FileSource* fileSource = FileSource::getNativePeer(env, jFileSource);
    assert(fileSource != nullptr);
    return *fileSource->fileSource;
}

void FileSource::registerNative(jni::JNIEnv& env) {
    //Register classes
    FileSource::javaClass = *jni::Class<FileSource>::Find(env).NewGlobalRef(env).release();
    FileSource::ResourceTransformCallback::javaClass = *jni::Class<FileSource::ResourceTransformCallback>::Find(env).NewGlobalRef(env).release();

    #define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<FileSource>(
        env, FileSource::javaClass, "nativePtr",
        std::make_unique<FileSource, JNIEnv&, jni::String, jni::String>,
        "initialize",
        "finalize",
        METHOD(&FileSource::setResourceTransform, "setResourceTransform")
    );
}


} // namespace android
} // namespace mbgl