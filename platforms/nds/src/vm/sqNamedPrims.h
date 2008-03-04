/* This is an automatically generated table of all builtin modules in the VM */

extern sqExport vm_exports[];
extern sqExport os_exports[];
extern sqExport FilePlugin_exports[];
extern sqExport MiscPrimitivePlugin_exports[];
extern sqExport SecurityPlugin_exports[];
extern sqExport BitBltPlugin_exports[];
extern sqExport LargeIntegers_exports[];

sqExport *pluginExports[] = {
	vm_exports,
	os_exports,
	FilePlugin_exports,
	MiscPrimitivePlugin_exports,
    SecurityPlugin_exports,
    BitBltPlugin_exports,
    LargeIntegers_exports,
	NULL
};
