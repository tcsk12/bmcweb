#include "app.hpp"
#include "dbus_utility.hpp"
#include "http_response.hpp"
#include "utils/dbus_utils.hpp"

namespace redfish
{

inline void requestRoutesSoftwareInventoryCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/UpdateService/SoftwareInventory/")
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
            {
                asyncResp->res.jsonValue["@odata.type"] = "#SoftwareInventoryCollection.SoftwareInventoryCollection";
                asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1/UpdateService/SoftwareInventory";
                asyncResp->res.jsonValue["Name"] = "Software Inventory Collection";
                asyncResp->res.jsonValue["Description"] = "Collection of software inventory entries";
                asyncResp->res.jsonValue["Members@odata.count"] = 1;
                asyncResp->res.jsonValue["Members"] = {
                    {{"@odata.id", "/redfish/v1/UpdateService/SoftwareInventory/BIOS/"}}
                };
            });
}

inline void requestRoutesSoftwareInventory(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/UpdateService/SoftwareInventory/BIOS/")
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
            {
                asyncResp->res.jsonValue["@odata.type"] = "#SoftwareInventory.v1_1_0.SoftwareInventory";
                asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1/UpdateService/SoftwareInventory/BIOS/";
                asyncResp->res.jsonValue["Id"] = "BIOS";
                asyncResp->res.jsonValue["Name"] = "BIOS Firmware";
                asyncResp->res.jsonValue["Description"] = "BIOS software image";

                constexpr std::string_view biosObjectPath = "/xyz/openbmc_project/software/bios_active";
                constexpr std::string_view biosService = "xyz.openbmc_project.Software.BIOS";

                crow::connections::systemBus->async_method_call(
                    [asyncResp](const boost::system::error_code ec,
                                const std::variant<std::vector<std::pair<std::string, std::variant<std::string>>>>& properties)
                    {
                        if (ec)
                        {
                            BMCWEB_LOG_ERROR("D-Bus GetAll error: {}", ec.message());
                            asyncResp->res.result(boost::beast::http::status::internal_server_error);
                            return;
                        }

                        const auto* props = std::get_if<std::vector<std::pair<std::string, std::variant<std::string>>>>(&properties);
                        if (!props)
                        {
                            BMCWEB_LOG_ERROR("Failed to get D-Bus properties!");
                            asyncResp->res.result(boost::beast::http::status::internal_server_error);
                            return;
                        }

                        for (const auto& [key, value] : *props)
                        {
                            if (key == "Version")
                            {
                                if (const std::string* val = std::get_if<std::string>(&value))
                                {
                                    asyncResp->res.jsonValue["Version"] = *val;
                                }
                            }
                            else if (key == "Manufacturer")
                            {
                                if (const std::string* val = std::get_if<std::string>(&value))
                                {
                                    asyncResp->res.jsonValue["Manufacturer"] = *val;
                                }
                            }
                            else if (key == "ReleaseDate")
                            {
                                if (const std::string* val = std::get_if<std::string>(&value))
                                {
                                    asyncResp->res.jsonValue["ReleaseDate"] = *val;
                                }
                            }
                        }

                        if (!asyncResp->res.jsonValue.contains("Version"))
                        {
                            asyncResp->res.jsonValue["Version"] = "Unknown";
                        }
                        if (!asyncResp->res.jsonValue.contains("Manufacturer"))
                        {
                            asyncResp->res.jsonValue["Manufacturer"] = "OpenFirmware Inc.";
                        }
                        if (!asyncResp->res.jsonValue.contains("ReleaseDate"))
                        {
                            asyncResp->res.jsonValue["ReleaseDate"] = "2024-11-01T00:00:00Z";
                        }

                        asyncResp->res.jsonValue["SoftwareId"] = "1234-BIOS";
                        asyncResp->res.jsonValue["Status"]["State"] = "Enabled";
                        asyncResp->res.jsonValue["Status"]["Health"] = "OK";
                        asyncResp->res.jsonValue["Updateable"] = true;
                        asyncResp->res.jsonValue["WriteProtected"] = false;
                        asyncResp->res.jsonValue["LowestSupportedVersion"] = "1.0.0";
                        asyncResp->res.jsonValue["UefiDevicePaths"] = nlohmann::json::array();
                        asyncResp->res.jsonValue["RelatedItem"] = {
                            {{"@odata.id", "/redfish/v1/Systems/system"}}
                        };
                        asyncResp->res.jsonValue["Actions"] = {
                            {"#SoftwareInventory.Update",
                             {{"target", "/redfish/v1/UpdateService/SoftwareInventory/BIOS/Actions/SoftwareInventory.Update"}}}
                        };
                    },
                    biosService.data(),
                    biosObjectPath.data(),
                    "org.freedesktop.DBus.Properties",
                    "GetAll",
                    "xyz.openbmc_project.Software.Version");
            });
}

} // namespace redfish

