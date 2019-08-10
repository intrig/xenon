basic_xv = files('basic.xv')
test('basic', xv_exe,
    workdir : meson.current_source_dir(),
    args : 'basic.xv')

test('brass', xv_exe,
    workdir : meson.current_source_dir(),
    args : ['--xddl_path=' + meson.source_root() + '/xddl', 'icd_brass.xv'])

xddl_path = meson.source_root() + '/xddl;' + meson.build_root() + '/xddl'

test('gold', xv_exe,
    workdir : meson.current_source_dir(),
    env : 'XDDLPATH=' + xddl_path,
    args : 'icd_gold.xv')
