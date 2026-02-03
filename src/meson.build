src_files += files('tmap.c', 'tmap.h')

inc_dirs += include_directories('.')


include_dir = get_option('prefix') / get_option('includedir')


install_headers(
    files('tmap.h'),
    install_dir: include_dir / 'tmap',
    preserve_path: true,
)

