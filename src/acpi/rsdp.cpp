#include <rsdp.hpp>
#include <multiboot.hpp>

namespace acpi {
    RSDPv1* RSDPManager::v1;
    RSDPv2* RSDPManager::v2;
    Uint8 RSDPManager::version;
    bool RSDPManager::initialized;

    void RSDPManager::init(multiboot::BootInfoHeader* header) {
        version = 0;
        multiboot::BootInfoTag* tag = header->firstTag;
        while (!tag->isTerminator()) {
            multiboot::BootInfoTagType type = tag->type;
            if (type == multiboot::BootInfoTagType::ACPIWithOldRSDP) {
                if (version == 2) {
                    continue;
                }
                version = 1;
                v1 = (RSDPv1*)(tag->as<multiboot::ACPIWithOldRSDPTag>()->table);
            } else if (type == multiboot::BootInfoTagType::ACPIWithNewRSDP) {
                version = 2;
                v2 = (RSDPv2*)(tag->as<multiboot::ACPIWithNewRSDPTag>()->table);
                v1 = (RSDPv1*)v2;
            }
            tag = tag->next();
        }
        if (version != 0) {
            initialized = true;
        }
    }

};