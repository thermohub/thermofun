print(lib_path..'\n')
print(ijsdata..'\n')


-- load  JSON4Lua module
  dofile(lib_path..'json.lua')
  json = require('json')

-- Now JSON decode the json string
   rintable = json.decode(ijsdata)

-- Make changes into structure  
   print( rintable.properties.element_symbol)
   rintable.properties.isotope_mass = 55
   
-- Encodes a table
   ojsdata = json.encode(rintable)
