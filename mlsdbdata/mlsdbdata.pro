TEMPLATE=aux

# generated with: geoclue-mlsdb-tool -r Devel MLS-full-cell-export-2016-03-14T000000.csv
devel_data.files=devel/mlsdb.data
devel_data.path=/usr/share/geoclue-provider-mlsdb/devel/

# India: generated with: geoclue-mlsdb-tool -c India MLS-full-cell-export-2016-03-14T000000.csv
india_data.files=in/mlsdb.data
india_data.path=/usr/share/geoclue-provider-mlsdb/in/

# Australia: generated with: geoclue-mlsdb-tool -c Australia MLS-full-cell-export-2016-03-14T000000.csv
australia_data.files=au/mlsdb.data
australia_data.path=/usr/share/geoclue-provider-mlsdb/au/

# Finland: generated with: geoclue-mlsdb-tool -c Finland MLS-full-cell-export-2016-03-14T000000.csv
finland_data.files=fi/mlsdb.data
finland_data.path=/usr/share/geoclue-provider-mlsdb/fi/

OTHER_FILES += \
    $$devel_data.files \
    $$india_data.files \
    $$australia_data.files \
    $$finland_data.files

INSTALLS += devel_data india_data australia_data finland_data
