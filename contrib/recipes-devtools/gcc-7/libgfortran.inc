require gcc-configure-common.inc

EXTRA_OECONF_PATHS = "\
    --with-sysroot=/not/exist \
    --with-build-sysroot=${STAGING_DIR_TARGET} \
"

do_configure () {
	for target in libbacktrace libgfortran
	do
		rm -rf ${B}/${TARGET_SYS}/$target/
		mkdir -p ${B}/${TARGET_SYS}/$target/
		cd ${B}/${TARGET_SYS}/$target/
		chmod a+x ${S}/$target/configure
		relpath=${@os.path.relpath("${S}", "${B}/${TARGET_SYS}")}
		../$relpath/$target/configure ${CONFIGUREOPTS} ${EXTRA_OECONF}
		# Easiest way to stop bad RPATHs getting into the library since we have a
		# broken libtool here
		sed -i -e 's/hardcode_into_libs=yes/hardcode_into_libs=no/' ${B}/${TARGET_SYS}/$target/libtool
	done
}
EXTRACONFFUNCS += "extract_stashed_builddir"
do_configure[depends] += "${COMPILERDEP}"

do_compile () {
	for target in libbacktrace libgfortran
	do
		cd ${B}/${TARGET_SYS}/$target/
		oe_runmake MULTIBUILDTOP=${B}/${TARGET_SYS}/$target/
	done
}

do_install () {
	cd ${B}/${TARGET_SYS}/libgfortran/
	oe_runmake 'DESTDIR=${D}' MULTIBUILDTOP=${B}/${TARGET_SYS}/libgfortran/ install
	if [ -d ${D}${libdir}/gcc/${TARGET_SYS}/${BINV}/finclude ]; then
		rmdir --ignore-fail-on-non-empty -p ${D}${libdir}/gcc/${TARGET_SYS}/${BINV}/finclude
	fi
	if [ -d ${D}${infodir} ]; then
		rmdir --ignore-fail-on-non-empty -p ${D}${infodir}
	fi
	chown -R root:root ${D}
}

INHIBIT_DEFAULT_DEPS = "1"
DEPENDS = "gcc-runtime gcc-cross-${TARGET_ARCH}"

BBCLASSEXTEND = "nativesdk"

PACKAGES = "\
    ${PN}-dbg \
    libgfortran \
    libgfortran-dev \
    libgfortran-staticdev \
"
FILES:${PN} = "${libdir}/libgfortran.so.*"
FILES:${PN}-dev = "\
    ${libdir}/libgfortran*.so \
    ${libdir}/libgfortran.spec \
    ${libdir}/libgfortran.la \
    ${libdir}/gcc/${TARGET_SYS}/${BINV}/libgfortranbegin.* \
    ${libdir}/gcc/${TARGET_SYS}/${BINV}/libcaf_single* \
    ${libdir}/gcc/${TARGET_SYS}/${BINV}/finclude/ \
"
FILES:${PN}-staticdev = "${libdir}/libgfortran.a"

INSANE_SKIP:${MLPREFIX}libgfortran-dev = "staticdev"

do_package_write_ipk[depends] += "virtual/${MLPREFIX}libc:do_packagedata"
do_package_write_deb[depends] += "virtual/${MLPREFIX}libc:do_packagedata"
do_package_write_rpm[depends] += "virtual/${MLPREFIX}libc:do_packagedata"

python __anonymous () {
    f = d.getVar("FORTRAN")
    if "fortran" not in f:
        raise bb.parse.SkipRecipe("libgfortran needs fortran support to be enabled in the compiler")
}
