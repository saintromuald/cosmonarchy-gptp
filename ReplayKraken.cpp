#include "pch.h"
#include "ReplayKraken.h"

// NOTE: This file is not part of unity build,
//       so httplib doesn't see Windows.h includes we have everywhere,
//       which would lead to compiler errors
#include <httplib.h>

namespace ReplayKraken {

bool send_replay(const InsertMatchRequest& request) {
    httplib::Client client{"https://cosmonarchy.com/api/games"};
    const Json json(request);
    const auto result = client.Post(json.dump());
    return result.error() == httplib::Error::Success;
}

} // namespace ReplayKraken
