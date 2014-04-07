# Copyright 2008 Charles Wang
# Distributed under the terms of the GNU General Public License v2

inherit eutils

DESCRIPTION="Coco & CocoXml implemented in C. Can be used to generate parser/scanner for other language too."
HOMEPAGE="http://code.google.com/p/cocoxml"
SRC_URI="http://cocoxml.googlecode.com/files/cocoxml-0.9.2.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="alpha amd64 ppha ia64 ~mips ppc ppc64 sparc x86"
IUSE=""

DEPEND=""

S="${WORKDIR}/${PN}-${PV}"

src_compile() {
  cd "${S}"
  cp configure.py configure
  econf || die "./configure failed"
  scons
}

src_install() {
  ./install.py DESTROOT=${D} install || die
}