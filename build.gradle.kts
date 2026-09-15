import org.gradle.internal.os.OperatingSystem

plugins {
    java
}

group = "net.meowsers"
version = "1.0"

val lwjglVersion = "3.4.3"
val jomlVersion = "1.10.9"
val imguiVersion = "1.92.7.1"

val lwjglNatives: String = run {
    val currentOs = OperatingSystem.current()
    val osArch = System.getProperty("os.arch")

    when {
        currentOs.isLinux -> {
            var natives = "natives-linux"
            if (osArch.startsWith("arm") || osArch.startsWith("aarch64")) {
                natives += if (osArch.contains("64") || osArch.startsWith("armv8")) "-arm64" else "-arm32"
            } else if (osArch.startsWith("ppc")) {
                natives += "-ppc64le"
            } else if (osArch.startsWith("riscv")) {
                natives += "-riscv64"
            }
            natives
        }
        currentOs.isMacOsX -> {
            if (osArch == "aarch64" || osArch == "arm64") "natives-macos-arm64" else "natives-macos"
        }
        currentOs.isWindows -> {
            if (osArch.contains("64")) {
                "natives-windows${if (osArch.startsWith("aarch64")) "-arm64" else ""}"
            } else {
                "natives-windows-x86"
            }
        }
        else -> error("Unsupported operating system: $currentOs")
    }
}
repositories {
    mavenCentral()
}

dependencies {
    implementation(platform("org.lwjgl:lwjgl-bom:$lwjglVersion"))

    implementation("org.lwjgl:lwjgl")
    implementation("org.lwjgl:lwjgl-assimp")
    implementation("org.lwjgl:lwjgl-freetype")
    implementation("org.lwjgl:lwjgl-glfw")
    implementation("org.lwjgl:lwjgl-jawt")
    implementation("org.lwjgl:lwjgl-nfd")
    implementation("org.lwjgl:lwjgl-openal")
    implementation("org.lwjgl:lwjgl-opengl")
    implementation("org.lwjgl:lwjgl-stb")

    runtimeOnly("org.lwjgl:lwjgl::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-assimp::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-freetype::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-glfw::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-nfd::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-openal::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-opengl::$lwjglNatives")
    runtimeOnly("org.lwjgl:lwjgl-stb::$lwjglNatives")

    implementation("org.joml:joml:$jomlVersion")
    implementation("io.github.spair:imgui-java-binding:$imguiVersion")
    implementation("io.github.spair:imgui-java-lwjgl3:$imguiVersion")
    val imguiOs = when {
        OperatingSystem.current().isMacOsX -> "macos"
        OperatingSystem.current().isWindows -> "windows"
        else -> "linux"
    }
    runtimeOnly("io.github.spair:imgui-java-natives-$imguiOs:$imguiVersion")

    testImplementation(platform("org.junit:junit-bom:5.11.4"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testRuntimeOnly("org.junit.platform:junit-platform-launcher")


}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(21))
    }
}
tasks.withType<JavaExec>().configureEach {
    if (OperatingSystem.current().isMacOsX) jvmArgs("-XstartOnFirstThread")
    jvmArgs("--enable-native-access=ALL-UNNAMED")
}
tasks.register<JavaExec>("runPeach") {
    group = "application"
    description = "Runs the engine"
    classpath = sourceSets.main.get().runtimeClasspath
    mainClass.set("net.meowsers.Main")
}

