#include <ad/MapManager.hpp>
namespace sdv::hpc::ad {
MapManager::MapManager(std::string mapPath) : mapPath_(std::move(mapPath)) {}
MapTile MapManager::tileAt(const Pose&) { return MapTile{}; }
}
