def generate_kml_grid(center_lat, center_lon, tile_size_deg, rows, cols):
    kml_header = '''<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <Folder>
      <name>SantiagoGrid</name>'''

    kml_footer = '''
    </Folder>
  </Document>
</kml>'''

    placemarks = ""
    for r in range(rows):
        for c in range(cols):
            lat = center_lat + (r - rows // 2) * tile_size_deg
            lon = center_lon + (c - cols // 2) * tile_size_deg
            placemarks += f'''
      <Placemark>
        <name>Chunk_{r}_{c}</name>
        <LookAt>
          <longitude>{lon}</longitude>
          <latitude>{lat}</latitude>
          <altitude>0</altitude>
          <range>500</range>
          <tilt>0</tilt>
          <heading>0</heading>
          <altitudeMode>relativeToGround</altitudeMode>
        </LookAt>
      </Placemark>'''

    return kml_header + placemarks + kml_footer

# Example: 6x6 grid of ~500m tiles (tile_size_deg ≈ 0.0045)
kml = generate_kml_grid(42.8782, -8.5448, 0.0045, 6, 6)
with open("santiago_grid.kml", "w") as f:
    f.write(kml)
