import argparse
from sys import argv

class GDExtensionGenerator:
    def __init__(self) -> None:
        self.text = ""

    def generate_section(self, title: str):
        if title == None: title = ""
        self.text += f"[{title}]\n" if len(title) != 0 else ""
        return self

    def generate_entry(self, key: str, value: str):
        if key == None: key = ""
        if value == None: value = ""
        self.text += f"{key} = {value}\n" if len(value) != 0 and len(key) != 0 else ""
        return self

    def generate_quoted_entry(self, key: str, value: str):
        if key == None: key = ""
        if value == None: value = ""
        self.text += f"{key} = \"{value}\"\n" if len(value) != 0 and len(key) != 0 else ""
        return self

def main():
    parser = argparse.ArgumentParser(
            prog=argv[0],
            description="Generate `.gdextension` configuration file."
            )
    parser.add_argument("entry_symbol", help="Entry symbol for the library.")
    parser.add_argument("compatibility_minimum", help="Minimum version for Godot to load GDExtension.")
    parser.add_argument("output", help="Output path.")
    parser.add_argument("-M", "--macos-release", help="Library path of release version for MacOS.")
    parser.add_argument("-m", "--macos-debug", help="Library path of debug version for MacOS.")
    parser.add_argument("-W", "--windows-64-release", help="Library path of release version for Windows x86_64.")
    parser.add_argument("-w", "--windows-64-debug", help="Library path of debug version for Windows x86_64.")
    parser.add_argument("-X", "--windows-32-release", help="Library path of release version for Windows x86_32.")
    parser.add_argument("-x", "--windows-32-debug", help="Library path of debug version for Windows x86_32.")
    parser.add_argument("-L", "--linux-release", help="Library path of release version for Linux x86_64.")
    parser.add_argument("-l", "--linux-debug", help="Library path of debug version for Linux x86_64.")
    parser.add_argument("-A", "--linux-arm64-release", help="Library path of release version for Linux arm64.")
    parser.add_argument("-a", "--linux-arm64-debug", help="Library path of debug version for Linux arm64.")
    parser.add_argument("-R", "--linux-rv64-release", help="Library path of release version for Linux rv64.")
    parser.add_argument("-r", "--linux-rv64-debug", help="Library path of debug version for Linux rv64.")
    parser.add_argument("-D", "--android-arm64-release", help="Library path of release version for Android arm64.")
    parser.add_argument("-d", "--android-arm64-debug", help="Library path of debug version for Android arm64.")
    parser.add_argument("-E", "--android-64-release", help="Library path of release version for Android x86_64.")
    parser.add_argument("-e", "--android-64-debug", help="Library path of debug version for Android x86_64.")

    args = parser.parse_args()
    with open(args.output, "w") as file:
        generator = GDExtensionGenerator()

        generator.generate_section("configuration") \
            .generate_quoted_entry("entry_symbol", args.entry_symbol) \
            .generate_entry("compatibility_minimum", args.compatibility_minimum) \
            .generate_section("libraries") \
            .generate_quoted_entry("macos.debug", args.macos_debug) \
            .generate_quoted_entry("macos.release", args.macos_release) \
            .generate_quoted_entry("windows.debug.x86_32", args.windows_32_debug) \
            .generate_quoted_entry("windows.release.x86_32", args.windows_32_release) \
            .generate_quoted_entry("windows.debug.x86_64", args.windows_64_debug) \
            .generate_quoted_entry("windows.release.x86_64", args.windows_64_release) \
            .generate_quoted_entry("linux.debug.x86_64", args.linux_debug) \
            .generate_quoted_entry("linux.release.x86_64", args.linux_release) \
            .generate_quoted_entry("linux.debug.arm64", args.linux_arm64_debug) \
            .generate_quoted_entry("linux.release.arm64", args.linux_arm64_release) \
            .generate_quoted_entry("linux.debug.rv64", args.linux_rv64_debug) \
            .generate_quoted_entry("linux.release.rv64", args.linux_rv64_release) \
            .generate_quoted_entry("android.debug.x86_64", args.android_64_debug) \
            .generate_quoted_entry("android.release.x86_64", args.android_64_release) \
            .generate_quoted_entry("android.debug.arm64", args.android_arm64_debug) \
            .generate_quoted_entry("android.release.arm64", args.android_arm64_release)

        file.write(generator.text)

    return 0

if __name__ == "__main__": exit(main())
