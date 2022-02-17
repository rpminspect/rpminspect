# Build libxdiff
libxdiff_sources = [
    'xdiffi.c',
    'xemit.c',
    'xhistogram.c',
    'xmerge.c',
    'xpatience.c',
    'xprepare.c',
    'xutils.c',
]

libxdiff = static_library(
    'xdiff',
    libxdiff_sources,
    include_directories : inc,
    install : false,
)
