class Wiredpanda < Formula
  desc "Logic circuit simulator designed to help students learn digital logic"
  homepage "https://gibis-unifesp.github.io/wiRedPanda/"
  url "https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/refs/tags/5.2.2.tar.gz"
  sha256 "f099a92d7ac4e7babce79b293076946e78eb53830e90a6be26184f368d2ea398"
  license "GPL-3.0-or-later"
  head "https://github.com/GIBIS-UNIFESP/wiRedPanda.git", branch: "master"

  livecheck do
    url :stable
    regex(/^v?(\d+(?:\.\d+)+)$/i)
  end

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "nlohmann-json" => :build
  depends_on "qtbase"
  depends_on "qtmultimedia"
  depends_on "qtsvg"
  depends_on "qttools" => :build

  resource "json-schema-validator" do
    url "https://github.com/pboettch/json-schema-validator/archive/refs/tags/2.4.0.tar.gz"
    sha256 "2ea2a4c3fbadb3fb60f9dabdd3bb552d4211385159289ef3427cfbf7168e5cf2"
  end

  def install
    validator_prefix = buildpath/"deps/json-schema-validator"
    resource("json-schema-validator").stage do
      system "cmake", "-S", ".", "-B", "build", "-G", "Ninja",
                      "-DBUILD_SHARED_LIBS=OFF",
                      "-DJSON_VALIDATOR_BUILD_TESTS=OFF",
                      "-DJSON_VALIDATOR_BUILD_EXAMPLES=OFF",
                      *std_cmake_args(install_prefix: validator_prefix)
      system "cmake", "--build", "build"
      system "cmake", "--install", "build"
    end

    args = [
      "-G", "Ninja",
      "-DWIREDPANDA_USE_SYSTEM_JSON=ON",
      "-DBUILD_TESTING=OFF",
      "-DCMAKE_PREFIX_PATH=#{validator_prefix}",
    ]
    args << "-DCMAKE_OSX_ARCHITECTURES=#{Hardware::CPU.arch}" if OS.mac?

    system "cmake", "-S", ".", "-B", "build", *args, *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"

    if OS.mac?
      bin.write_exec_script prefix/"wiredpanda.app/Contents/MacOS/wiredpanda"
    end
  end

  test do
    ENV["QT_QPA_PLATFORM"] = "offscreen"
    assert_match version.to_s, shell_output("#{bin}/wiredpanda --version")
  end
end
