plugins {
    java
    application
}

group = "com.engine"
version = "1.0.0"

repositories {
    mavenCentral()
}

dependencies {
    implementation("org.joml:joml:1.10.7")
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(21))
    }
}

tasks.withType<JavaExec>().configureEach {
    jvmArgs("-XstartOnFirstThread", "--enable-native-access=ALL-UNNAMED", "--sun-misc-unsafe-memory-access=allow")
}


tasks.register<JavaExec>("runPeach") {
    group = "application"
    description = "Runs the engine"
    classpath = sourceSets.main.get().runtimeClasspath
    mainClass.set("net.meowsers.Main")
}

