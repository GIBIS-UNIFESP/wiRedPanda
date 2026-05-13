cask "wiredpanda" do
  version "5.0.1"
  sha256 "0000000000000000000000000000000000000000000000000000000000000000"

  url "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/#{version}/wiRedPanda-#{version}-macOS-Qt6.9.3.dmg",
      verified: "github.com/GIBIS-UNIFESP/wiRedPanda/"
  name "wiRedPanda"
  desc "Logic circuit simulator focused on education"
  homepage "https://github.com/GIBIS-UNIFESP/wiRedPanda"

  livecheck do
    url :url
    strategy :github_latest
  end

  app "wiredpanda.app"

  zap trash: [
    "~/Library/Preferences/com.wiredpanda.wiRedPanda.plist",
    "~/Library/Saved Application State/com.wiredpanda.wiRedPanda.savedState",
  ]
end
