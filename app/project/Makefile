# Update project with:
# $(ANDROID_SDK)/tools/android update project --name OgreClient --target android-23 --path .

PROTOBUF_JAR = protobuf-javanano-3.0.0-alpha-7.jar
OKHTTP_JAR = okhttp-3.10.0.jar
OKIO_JAR = okio-1.14.1.jar

all: libs/$(PROTOBUF_JAR) libs/$(OKHTTP_JAR) libs/$(OKIO_JAR) libs/armeabi-v7a/libgvr.so libs/armeabi-v7a/libogreclient.so
	ant -Dgvr.sdk=$(GVRSDK) -emacs debug

libs:
	mkdir -p libs

libs/$(PROTOBUF_JAR): libs
	wget -O $@ http://central.maven.org/maven2/com/google/protobuf/nano/protobuf-javanano/3.0.0-alpha-7/$(PROTOBUF_JAR)

libs/$(OKHTTP_JAR): libs
	wget -O $@ http://repo1.maven.org/maven2/com/squareup/okhttp3/okhttp/3.10.0/$(OKHTTP_JAR)

libs/$(OKIO_JAR): libs
	wget -O $@ http://repo1.maven.org/maven2/com/squareup/okio/okio/1.14.1/$(OKIO_JAR)

libs/armeabi-v7a/libgvr.so:
	mkdir -p libs/armeabi-v7a
	cp -f $(GVRNDK)/lib/android_arm/libgvr.so libs/armeabi-v7a/libgvr.so

libs/armeabi-v7a/libogreclient.so: ../cxx/ogreclient.so
	mkdir -p libs/armeabi-v7a
	cp -f ../cxx/ogreclient.so libs/armeabi-v7a/libogreclient.so

clean:
	ant clean
	rm -rf res.generated

.PHONY: clean all
