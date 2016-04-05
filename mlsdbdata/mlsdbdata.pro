TEMPLATE=aux

# generated with: geoclue-mlsdb-tool -r Devel MLS-full-cell-export-2016-03-14T000000.csv
devel_data.files=\
    devel/1/mlsdb.data \
    devel/2/mlsdb.data \
    devel/3/mlsdb.data \
    devel/4/mlsdb.data \
    devel/5/mlsdb.data \
    devel/6/mlsdb.data \
    devel/7/mlsdb.data \
    devel/8/mlsdb.data \
    devel/9/mlsdb.data
devel_data.path=/usr/share/geoclue-provider-mlsdb/devel/

# India: generated with: geoclue-mlsdb-tool -c India MLS-full-cell-export-2016-03-14T000000.csv
india_data.files=\
    in/1/mlsdb.data \
    in/2/mlsdb.data \
    in/3/mlsdb.data \
    in/4/mlsdb.data \
    in/5/mlsdb.data \
    in/6/mlsdb.data \
    in/7/mlsdb.data \
    in/8/mlsdb.data \
    in/9/mlsdb.data
india_data.path=/usr/share/geoclue-provider-mlsdb/in/

# Australia: generated with: geoclue-mlsdb-tool -c Australia MLS-full-cell-export-2016-03-14T000000.csv
australia_data.files=\
    au/1/mlsdb.data \
    au/2/mlsdb.data \
    au/3/mlsdb.data \
    au/4/mlsdb.data \
    au/5/mlsdb.data \
    au/6/mlsdb.data \
    au/7/mlsdb.data \
    au/8/mlsdb.data \
    au/9/mlsdb.data
australia_data.path=/usr/share/geoclue-provider-mlsdb/au/

# Finland: generated with: geoclue-mlsdb-tool -c Finland MLS-full-cell-export-2016-03-14T000000.csv
finland_data.files=\
    fi/1/mlsdb.data \
    fi/2/mlsdb.data \
    fi/3/mlsdb.data \
    fi/4/mlsdb.data \
    fi/5/mlsdb.data \
    fi/6/mlsdb.data \
    fi/7/mlsdb.data \
    fi/8/mlsdb.data \
    fi/9/mlsdb.data
finland_data.path=/usr/share/geoclue-provider-mlsdb/fi/

OTHER_FILES += \
    $$devel_data.files \
    $$india_data.files \
    $$australia_data.files \
    $$finland_data.files

INSTALLS += devel_data india_data australia_data finland_data
