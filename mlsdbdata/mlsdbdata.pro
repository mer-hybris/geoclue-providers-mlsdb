TEMPLATE=aux

# India:
india_data.files=data/404.* data/405.*
india_data.path=/usr/share/geoclue-provider-mlsdb/data/

# Australia:
australia_data.files=data/505.*
australia_data.path=/usr/share/geoclue-provider-mlsdb/data/

# Finland:
finland_data.files=data/244.*
finland_data.path=/usr/share/geoclue-provider-mlsdb/data/

OTHER_FILES += \
    $$india_data.files \
    $$australia_data.files \
    $$finland_data.files

INSTALLS += india_data australia_data finland_data
