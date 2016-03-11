TEMPLATE=aux

# generated with: geoclue-mlsdb-tool -r Devel MLS-full-cell-export-2016-03-14T000000.csv
devel_data.files=devel/mlsdb.data
devel_data.path=/usr/share/geoclue-provider-mlsdb/

# generated with: geoclue-mlsdb-tool -c India MLS-full-cell-export-2016-03-14T000000.csv
india_data.files=india/mlsdb.data
india_data.path=/usr/share/geoclue-provider-mlsdb/

# generated with: geoclue-mlsdb-tool -c Australia MLS-full-cell-export-2016-03-14T000000.csv
australia_data.files=australia/mlsdb.data
australia_data.path=/usr/share/geoclue-provider-mlsdb/

# generated with: geoclue-mlsdb-tool -c Finland MLS-full-cell-export-2016-03-14T000000.csv
finland_data.files=finland/mlsdb.data
finland_data.files=/usr/share/geoclue-provider-mlsdb/

OTHER_FILES += \
    $$devel_data.files \
    $$india_data.files \
    $$australia_data.files \
    $$finland_data.files

INSTALLS += devel_data
