#ifndef FORMATS_PBF_OSMPBFPARSER_HPP_INCLUDED
#define FORMATS_PBF_OSMPBFPARSER_HPP_INCLUDED

#include "BoundingBox.hpp"
#include "formats/FormatTypes.hpp"

#include <fileformat.pb.h>
#include <osmformat.pb.h>
#include <zlib.h>

#include <cstdint>
#include <istream>
#include <stdexcept>
#include <vector>

namespace utymap {
namespace formats {

template<typename Visitor>
class OsmPbfParser final {
  const static int MaxBlobHeaderSize = 64*1024;
  const static int MaxUncompressedBlobSize = 32*1024*1024;

 public:

  OsmPbfParser() :
      buffer_(MaxUncompressedBlobSize),
      unpack_buffer_(MaxUncompressedBlobSize),
      finished_(false) {
  }

  void parse(std::istream &stream, Visitor &visitor) {
    finished_ = false;

    while (!stream.eof() && !stream.fail() && !finished_) {
      OSMPBF::BlobHeader header = readHeader(stream);
      if (!finished_) {
        std::int32_t sz = readBlob(header, stream);
        if (header.type()=="OSMData") {
          parsePrimitiveBlock(sz, visitor);
        } else if (header.type()=="OSMHeader") {
          // used to be skipped
        }
      }
    }
  }

 private:

  std::vector<char> buffer_;
  std::vector<char> unpack_buffer_;
  bool finished_;

  OSMPBF::BlobHeader readHeader(std::istream &stream) {
    std::int32_t sz;
    OSMPBF::BlobHeader result;

    // read size of blob-header
    if (!stream.read(reinterpret_cast<char *>(&sz), 4)) {
      finished_ = true;
      return result;
    }

    // little endian to big endian
    sz = (((sz & 0xff) << 24) + ((sz & 0xff00) << 8) + ((sz & 0xff0000) >> 8) + ((sz >> 24) & 0xff));

    if (sz > MaxBlobHeaderSize)
      throw std::domain_error("Blob header size is bigger than allowed");

    stream.read(buffer_.data(), sz);
    if (!stream.good())
      throw std::domain_error("Unable to read blob header from file");

    if (!result.ParseFromArray(buffer_.data(), sz))
      throw std::domain_error("Unable to parse blob header");

    return result;
  }

  std::int32_t readBlob(const OSMPBF::BlobHeader &header, std::istream &stream) {
    OSMPBF::Blob blob;

    std::int32_t sz = header.datasize();

    if (sz > MaxUncompressedBlobSize)
      throw std::domain_error("Blob size is bigger then allowed");

    if (!stream.read(buffer_.data(), sz))
      throw std::domain_error("Unable to read blob from file");

    if (!blob.ParseFromArray(buffer_.data(), sz))
      throw std::domain_error("Unable to parse blob");

    // uncompressed
    if (blob.has_raw()) {
      sz = static_cast<std::int32_t>(blob.raw().size());
      memcpy(unpack_buffer_.data(), buffer_.data(), sz);
      return sz;
    }

    if (blob.has_zlib_data()) {
      sz = static_cast<std::int32_t>(blob.zlib_data().size());

      z_stream z;
      z.next_in = (unsigned char *) blob.zlib_data().c_str();
      z.avail_in = sz;
      z.next_out = reinterpret_cast<unsigned char *>(unpack_buffer_.data());
      z.avail_out = blob.raw_size();
      z.zalloc = Z_NULL;
      z.zfree = Z_NULL;
      z.opaque = Z_NULL;

      if (inflateInit(&z)!=Z_OK)
        throw std::domain_error("Failed to init zlib stream");

      if (inflate(&z, Z_FINISH)!=Z_STREAM_END)
        throw std::domain_error("Failed to inflate zlib stream");

      if (inflateEnd(&z)!=Z_OK)
        throw std::domain_error("Failed to deinit zlib stream");

      return z.total_out;
    }

    if (blob.has_lzma_data())
      throw std::domain_error("Lzma-decompression is not supported");

    return 0;
  }

  void parsePrimitiveBlock(int32_t sz, Visitor &visitor) {
    OSMPBF::PrimitiveBlock primblock;

    if (!primblock.ParseFromArray(unpack_buffer_.data(), sz))
      throw std::domain_error("Unable to parse primitive block");

    for (int i = 0, l = primblock.primitivegroup_size(); i < l; i++) {
      OSMPBF::PrimitiveGroup pg = primblock.primitivegroup(i);

      // simple nodes
      for (int i = 0; i < pg.nodes_size(); ++i) {
        OSMPBF::Node n = pg.nodes(i);
        GeoCoordinate coordinate;
        coordinate.latitude = 0.000000001*(primblock.lat_offset() + (primblock.granularity()*n.lat()));
        coordinate.longitude = 0.000000001*(primblock.lon_offset() + (primblock.granularity()*n.lon()));
        std::uint64_t id = n.id();
        Tags tags;
        setTags(n, primblock, tags);
        visitor.visitNode(id, coordinate, tags);
      }

      // dense nodes
      if (pg.has_dense()) {
        OSMPBF::DenseNodes dn = pg.dense();
        uint64_t id = 0;
        double lon = 0;
        double lat = 0;

        int current_kv = 0;

        for (int i = 0; i < dn.id_size(); ++i) {
          id += dn.id(i);
          lat += 0.000000001*(primblock.lat_offset() + (primblock.granularity()*dn.lat(i)));
          lon += 0.000000001*(primblock.lon_offset() + (primblock.granularity()*dn.lon(i)));

          Tags tags;
          tags.reserve(2);
          while (current_kv < dn.keys_vals_size() && dn.keys_vals(current_kv)!=0) {
            auto key = dn.keys_vals(current_kv);
            auto val = dn.keys_vals(current_kv + 1);
            Tag tag;
            tag.key = primblock.stringtable().s(key);
            tag.value = primblock.stringtable().s(val);
            current_kv += 2;
            tags.push_back(tag);
          }
          ++current_kv;
          GeoCoordinate coordinate(lat, lon);
          visitor.visitNode(id, coordinate, tags);
        }
      }

      for (int i = 0; i < pg.ways_size(); ++i) {
        OSMPBF::Way w = pg.ways(i);

        uint64_t ref = 0;
        std::vector<uint64_t> nodeIds;
        nodeIds.reserve(w.refs_size());
        for (int j = 0; j < w.refs_size(); ++j) {
          ref += w.refs(j);
          nodeIds.push_back(ref);
        }
        uint64_t id = w.id();
        Tags tags;
        setTags(w, primblock, tags);
        visitor.visitWay(id, nodeIds, tags);
      }

      for (int i = 0; i < pg.relations_size(); ++i) {
        OSMPBF::Relation rel = pg.relations(i);
        uint64_t id = 0;
        RelationMembers refs;
        refs.reserve(rel.memids_size());
        for (int l = 0; l < rel.memids_size(); ++l) {
          id += rel.memids(l);
          RelationMember member;
          member.refId = id;
          member.type = parseType(rel, l);
          member.role = primblock.stringtable().s(rel.roles_sid(l));
          refs.push_back(member);
        }

        uint64_t rel_id = rel.id();
        Tags tags;
        setTags(rel, primblock, tags);
        visitor.visitRelation(rel_id, refs, tags);
      }
    }
  }

  static std::string parseType(OSMPBF::Relation &rel, int index) {
    switch (rel.types(index)) {
      case OSMPBF::Relation::NODE:return "n";
      case OSMPBF::Relation::WAY:return "w";
      case OSMPBF::Relation::RELATION:
      default:return "r";
    }
  }

  template<typename T>
  void setTags(const T &object, const OSMPBF::PrimitiveBlock &primblock, Tags &tags) {
    tags.reserve(object.keys_size());
    for (int i = 0; i < object.keys_size(); ++i) {
      Tag tag;
      auto key = object.keys(i);
      auto val = object.vals(i);
      tag.key = primblock.stringtable().s(key);
      tag.value = primblock.stringtable().s(val);
      tags.push_back(tag);
    }
  }
};

}
}

#endif  // FORMATS_PBF_OSMPBFPARSER_HPP_INCLUDED
