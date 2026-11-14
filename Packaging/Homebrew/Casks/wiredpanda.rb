cask "wiredpanda" do
  version "5.2.2,6.9.3"
  sha256 "e1a92347c0ed23785e34d939f260a446ba1355470d196ab6bfb03d22b46ca83d"

  url "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/#{version.csv.first}/wiRedPanda-#{version.csv.first}-macOS-Qt#{version.csv.second}.dmg",
      verified: "github.com/GIBIS-UNIFESP/wiRedPanda/"
  name "wiRedPanda"
  desc "Logic circuit simulator"
  homepage "https://gibis-unifesp.github.io/wiRedPanda/"

  livecheck do
    url :url
    regex(/^wiRedPanda[._-]v?(\d+(?:\.\d+)+)[._-]macOS[._-]Qt(\d+(?:\.\d+)+)\.dmg$/i)
    strategy :github_latest do |json, regex|
      json["assets"]&.map do |asset|
        match = asset["name"]&.match(regex)
        next if match.blank?

        "#{match[1]},#{match[2]}"
      end
    end
  end

  depends_on macos: ">= :ventura"

  app "wiredpanda.app"
  binary "#{appdir}/wiredpanda.app/Contents/MacOS/wiredpanda"

  zap trash: [
    "~/Library/Preferences/com.gibis-unifesp.wiRedPanda.plist",
    "~/Library/Preferences/io.github.gibis_unifesp.wiRedPanda.plist",
    "~/Library/Saved Application State/io.github.gibis_unifesp.wiRedPanda.savedState",
  ]
end
