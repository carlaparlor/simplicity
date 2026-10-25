"""Read simple on-curve TrueType polygons from the pinned RetroByte font.
Only the subset required by this font's pixel s is supported. Other formats,
composite/curved glyphs and absent characters fail rather than approximate art.
"""
import struct


def contours(data, character='s'):
    if data[:4] != b'\0\1\0\0':
        raise ValueError('Expected TrueType sfnt')
    u16 = lambda p: struct.unpack_from('>H', data, p)[0]
    i16 = lambda p: struct.unpack_from('>h', data, p)[0]
    u32 = lambda p: struct.unpack_from('>I', data, p)[0]
    tables = {}
    for i in range(u16(4)):
        p = 12 + 16*i
        tables[data[p:p+4]] = u32(p+8)
    cmap = tables[b'cmap']
    sub = next((cmap+u32(cmap+8+8*i) for i in range(u16(cmap+2))
                if u16(cmap+u32(cmap+8+8*i)) == 4), None)
    if sub is None:
        raise ValueError('Expected format 4 cmap')
    segments = u16(sub+6)//2
    ends = sub+14
    starts = ends+2*segments+2
    deltas = starts+2*segments
    ranges = deltas+2*segments
    code = ord(character)
    glyph = 0
    for i in range(segments):
        if u16(starts+2*i) <= code <= u16(ends+2*i):
            r = u16(ranges+2*i)
            glyph = (code+i16(deltas+2*i)) % 65536 if r == 0 else u16(ranges+2*i+r+2*(code-u16(starts+2*i)))
            if r and glyph:
                glyph = (glyph+i16(deltas+2*i)) % 65536
            break
    if glyph == 0:
        raise ValueError('Character missing')
    loca = tables[b'loca']
    offset = u32(loca+4*glyph) if i16(tables[b'head']+50) else 2*u16(loca+2*glyph)
    start = tables[b'glyf']+offset
    count = i16(start)
    if count <= 0:
        raise ValueError('Expected simple nonempty glyph')
    end_points = [u16(start+10+2*i) for i in range(count)]
    p = start+10+2*count
    p += 2+u16(p)
    flags = []
    while len(flags) <= end_points[-1]:
        flag = data[p]; p += 1
        repeat = 1
        if flag & 8:
            repeat += data[p]; p += 1
        flags.extend([flag]*repeat)
    if len(flags) != end_points[-1]+1 or any(not flag & 1 for flag in flags):
        raise ValueError('Expected on-curve polygon points')
    coordinates = []
    for short, positive in ((2,16),(4,32)):
        values = []; value = 0
        for flag in flags:
            delta = 0
            if flag & short:
                delta = data[p] * (1 if flag & positive else -1); p += 1
            elif not flag & positive:
                delta = i16(p); p += 2
            value += delta; values.append(value)
        coordinates.append(values)
    points = list(zip(*coordinates))
    result = []; first = 0
    for end in end_points:
        result.append(points[first:end+1]); first = end+1
    return result
