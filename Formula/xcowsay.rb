class Xcowsay < Formula
  desc "Graphical talking cow"
  homepage "https://www.doof.me.uk/xcowsay"
  url "https://github.com/nickg/xcowsay/releases/download/v1.4/xcowsay-1.4.tar.gz"
  sha256 "c7e261ba0262c3821c106ccb6d6f984e3c2da999ad10151364e55d1c699f8e51"
  head "https://github.com/nickg/xcowsay.git"

  depends_on "autoconf" => :build
  depends_on "automake" => :build
  depends_on "fontconfig" => :build
  depends_on "gettext" => :build
  depends_on "pkg-config" => :build
  depends_on "gtk+"
  depends_on :x11

  def install
    system "./autogen.sh" if build.head?
    system "./configure", "--prefix=#{prefix}"
    system "make"
    system "make", "install"
  end

  test do
    assert_match version.to_s, shell_output("#{bin}/xcowsay -v")
  end
end
