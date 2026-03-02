# Loader Provenance

This directory contains a vendored copy of the Apple II ProDOS loader used for
autobooting the game disk:

- `loader.s`
- `loader.cfg`

## Origin

These files come from the cc65 project (`libsrc/apple2/targetutil`) and are based
on `LOADER.SYSTEM`.

Original loader author:

- Oliver Schmidt

cc65 project website:

- https://cc65.github.io/

Apple II target documentation (loader section):

- https://cc65.github.io/doc/apple2.html#s5

## Local Modification

This repo patches the loader to load the fixed ProDOS filename `IIMPERIALISM`
so we can keep a short bootable `*.SYSTEM` filename (`IIMP.SYSTEM`) within ProDOS
filename length limits.
