/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

/**
 * Helper functions for creating xml strings used by application update tests.
 *
 * !IMPORTANT - This file contains everything needed (along with dependencies)
 * by the updates.sjs file used by the mochitest-chrome tests. Since xpcshell
 * used by the http server is launched with -v 170 this file must not use
 * features greater than JavaScript 1.7.
 */

const FILE_SIMPLE_MAR = "simple_no_pib.mar";

const SIZE_SIMPLE_MAR = "351";

const MD5_HASH_SIMPLE_MAR    = "d0a7f84dacc55a252ab916668a7cb216";
const SHA1_HASH_SIMPLE_MAR   = "f5053f9552d087c6c6ed83f9b19405eccf1436fc";
const SHA256_HASH_SIMPLE_MAR = "663c7cbd11fe45b0a71438387db924d205997ab85ccf5" +
                               "b40aebbdaef179796ab";
const SHA384_HASH_SIMPLE_MAR = "a57250554755a9f42b91932993599bb6b05e063dcbd71" +
                               "846e350232945dbad2b0c83208a07810cf798b3d11393" +
                               "99c453";
const SHA512_HASH_SIMPLE_MAR = "55d3e2a86acaeb0abb7a444c13bba748846fcbac7ff05" +
                               "8f8ee9c9260ba01e6aef86fa4a6c46a3016b675ef94e7" +
                               "7e63fbe912f64d155bed9b1c341dd56e575a26";

const STATE_NONE            = "null";
const STATE_DOWNLOADING     = "downloading";
const STATE_PENDING         = "pending";
const STATE_PENDING_SVC     = "pending-service";
const STATE_APPLYING        = "applying";
const STATE_SUCCEEDED       = "succeeded";
const STATE_DOWNLOAD_FAILED = "download-failed";
const STATE_FAILED          = "failed";

/**
 * Constructs a string representing a remote update xml file.
 *
 * @param  aUpdates
 *         The string representing the update elements.
 * @return The string representing a remote update xml file.
 */
function getRemoteUpdatesXMLString(aUpdates) {
  return "<?xml version=\"1.0\"?>\n" +
         "<updates>\n" +
           aUpdates +
         "</updates>\n";
}

/**
 * Constructs a string representing an update element for a remote update xml
 * file. See getUpdateString for parameter information not provided below.
 *
 * @param  aPatches
 *         String representing the application update patches.
 * @return The string representing an update element for an update xml file.
 */
function getRemoteUpdateString(aPatches, aType, aName, aDisplayVersion,
                               aAppVersion, aPlatformVersion, aBuildID,
                               aDetailsURL, aBillboardURL, aLicenseURL,
                               aShowPrompt, aShowNeverForVersion, aShowSurvey,
                               aVersion, aExtensionVersion, aCustom1,
                               aCustom2) {
  return getUpdateString(aType, aName, aDisplayVersion, aAppVersion,
                         aPlatformVersion, aBuildID, aDetailsURL,
                         aBillboardURL, aLicenseURL, aShowPrompt,
                         aShowNeverForVersion, aShowSurvey, aVersion,
                         aExtensionVersion, aCustom1, aCustom2) + ">\n" +
              aPatches +
         "  </update>\n";
}

/**
 * Constructs a string representing a patch element for a remote update xml
 * file. See getPatchString for parameter information not provided below.
 *
 * @return The string representing a patch element for a remote update xml file.
 */
function getRemotePatchString(aType, aURL, aHashFunction, aHashValue, aSize) {
  return getPatchString(aType, aURL, aHashFunction, aHashValue, aSize) +
         "/>\n";
}

/**
 * Constructs a string representing a local update xml file.
 *
 * @param  aUpdates
 *         The string representing the update elements.
 * @return The string representing a local update xml file.
 */
function getLocalUpdatesXMLString(aUpdates) {
  if (!aUpdates || aUpdates == "")
    return "<updates xmlns=\"http://www.mozilla.org/2005/app-update\"/>"
  return ("<updates xmlns=\"http://www.mozilla.org/2005/app-update\">" +
            aUpdates +
          "</updates>").replace(/>\s+\n*</g,'><');
}

/**
 * Constructs a string representing an update element for a local update xml
 * file. See getUpdateString for parameter information not provided below.
 *
 * @param  aPatches
 *         String representing the application update patches.
 * @param  aServiceURL (optional)
 *         The update's xml url.
 *         If not specified it will default to 'http://test_service/'.
 * @param  aIsCompleteUpdate (optional)
 *         The string 'true' if this update was a complete update or the string
 *         'false' if this update was a partial update.
 *         If not specified it will default to 'true'.
 * @param  aChannel (optional)
 *         The update channel name.
 *         If not specified it will default to 'test_channel'.
 * @param  aForegroundDownload (optional)
 *         The string 'true' if this update was manually downloaded or the
 *         string 'false' if this update was automatically downloaded.
 *         If not specified it will default to 'true'.
 * @param  aPreviousAppVersion (optional)
 *         The application version prior to applying the update.
 *         If not specified it will not be present.
 * @return The string representing an update element for an update xml file.
 */
function getLocalUpdateString(aPatches, aType, aName, aDisplayVersion,
                              aAppVersion, aPlatformVersion, aBuildID,
                              aDetailsURL, aBillboardURL, aLicenseURL,
                              aServiceURL, aInstallDate, aStatusText,
                              aIsCompleteUpdate, aChannel, aForegroundDownload,
                              aShowPrompt, aShowNeverForVersion, aShowSurvey,
                              aVersion, aExtensionVersion, aPreviousAppVersion,
                              aCustom1, aCustom2) {
  let serviceURL = aServiceURL ? aServiceURL : "http://test_service/";
  let installDate = aInstallDate ? aInstallDate : "1238441400314";
  let statusText = aStatusText ? aStatusText : "Install Pending";
  let isCompleteUpdate =
    typeof(aIsCompleteUpdate) == "string" ? aIsCompleteUpdate : "true";
  let channel = aChannel ? aChannel : "test_channel";
  let foregroundDownload =
    typeof(aForegroundDownload) == "string" ? aForegroundDownload : "true";
  let previousAppVersion = aPreviousAppVersion ? "previousAppVersion=\"" +
                                                 aPreviousAppVersion + "\" "
                                               : "";
  return getUpdateString(aType, aName, aDisplayVersion, aAppVersion,
                         aPlatformVersion, aBuildID, aDetailsURL, aBillboardURL,
                         aLicenseURL, aShowPrompt, aShowNeverForVersion,
                         aShowSurvey, aVersion, aExtensionVersion, aCustom1,
                         aCustom2) +
                   " " +
                   previousAppVersion +
                   "serviceURL=\"" + serviceURL + "\" " +
                   "installDate=\"" + installDate + "\" " +
                   "statusText=\"" + statusText + "\" " +
                   "isCompleteUpdate=\"" + isCompleteUpdate + "\" " +
                   "channel=\"" + channel + "\" " +
                   "foregroundDownload=\"" + foregroundDownload + "\">"  +
              aPatches +
         "  </update>";
}

/**
 * Constructs a string representing a patch element for a local update xml file.
 * See getPatchString for parameter information not provided below.
 *
 * @param  aSelected (optional)
 *         Whether this patch is selected represented or not. The string 'true'
 *         denotes selected and the string 'false' denotes not selected.
 *         If not specified it will default to the string 'true'.
 * @param  aState (optional)
 *         The patch's state.
 *         If not specified it will default to STATE_SUCCEEDED.
 * @return The string representing a patch element for a local update xml file.
 */
function getLocalPatchString(aType, aURL, aHashFunction, aHashValue, aSize,
                             aSelected, aState) {
  let selected = typeof(aSelected) == "string" ? aSelected : "true";
  let state = aState ? aState : STATE_SUCCEEDED;
  return getPatchString(aType, aURL, aHashFunction, aHashValue, aSize) + " " +
         "selected=\"" + selected + "\" " +
         "state=\"" + state + "\"/>\n";
}

/**
 * Constructs a string representing an update element for a remote update xml
 * file.
 *
 * @param  aType (optional)
 *         The update's type which should be major or minor. If not specified it
 *         will default to 'major'.
 * @param  aName (optional)
 *         The update's name.
 *         If not specified it will default to 'App Update Test'.
 * @param  aDisplayVersion (optional)
 *         The update's display version.
 *         If not specified it will default to 'version #' where # is the value
 *         of DEFAULT_UPDATE_VERSION.
 * @param  aAppVersion (optional)
 *         The update's application version.
 *         If not specified it will default to the value of
 *         DEFAULT_UPDATE_VERSION.
 * @param  aPlatformVersion (optional)
 *         The update's platform version.
 *         If not specified it will default to the value of
 *         DEFAULT_UPDATE_VERSION.
 * @param  aBuildID (optional)
 *         The update's build id.
 *         If not specified it will default to '20080811053724'.
 * @param  aDetailsURL (optional)
 *         The update's details url.
 *         If not specified it will default to 'http://test_details/' due to due
 *         to bug 470244.
 * @param  aBillboardURL (optional)
 *         The update's billboard url.
 *         If not specified it will not be present.
 * @param  aLicenseURL (optional)
 *         The update's license url.
 *         If not specified it will not be present.
 * @param  aShowPrompt (optional)
 *         Whether to show the prompt for the update when auto update is
 *         enabled.
 *         If not specified it will not be present and the update service will
 *         default to false.
 * @param  aShowNeverForVersion (optional)
 *         Whether to show the 'No Thanks' button in the update prompt.
 *         If not specified it will not be present and the update service will
 *         default to false.
 * @param  aShowSurvey (optional)
 *         Whether to show the 'No Thanks' button in the update prompt.
 *         If not specified it will not be present and the update service will
 *         default to false.
 * @param  aVersion (optional)
 *         The update's application version from 1.9.2.
 *         If not specified it will not be present.
 * @param  aExtensionVersion (optional)
 *         The update's application version from 1.9.2.
 *         If not specified it will not be present.
 * @param  aCustom1 (optional)
 *         A custom attribute name and attribute value to add to the xml.
 *         Example: custom1_attribute="custom1 value"
 *         If not specified it will not be present.
 * @param  aCustom2 (optional)
 *         A custom attribute name and attribute value to add to the xml.
 *         Example: custom2_attribute="custom2 value"
 *         If not specified it will not be present.
 * @return The string representing an update element for an update xml file.
 */
function getUpdateString(aType, aName, aDisplayVersion, aAppVersion,
                         aPlatformVersion, aBuildID, aDetailsURL, aBillboardURL,
                         aLicenseURL, aShowPrompt, aShowNeverForVersion,
                         aShowSurvey, aVersion, aExtensionVersion, aCustom1,
                         aCustom2) {
  let type = aType ? aType : "major";
  let name = aName ? aName : "App Update Test";
  let displayVersion = "";
  if (aDisplayVersion || !aVersion) {
    displayVersion = "displayVersion=\"" +
                     (aDisplayVersion ? aDisplayVersion
                                      : "version " + DEFAULT_UPDATE_VERSION) +
                     "\" ";
  }
  // version has been deprecated in favor of displayVersion but it still needs
  // to be tested for forward compatibility.
  let version = aVersion ? "version=\"" + aVersion + "\" " : "";
  let appVersion = "";
  if (aAppVersion || !aExtensionVersion) {
    appVersion = "appVersion=\"" +
                 (aAppVersion ? aAppVersion : DEFAULT_UPDATE_VERSION) +
                 "\" ";
  }
  // extensionVersion has been deprecated in favor of appVersion but it still
  // needs to be tested for forward compatibility.
  let extensionVersion = aExtensionVersion ? "extensionVersion=\"" +
                                             aExtensionVersion + "\" "
                                           : "";
  let platformVersion = "";
  if (aPlatformVersion) {
    platformVersion = "platformVersion=\"" +
                      (aPlatformVersion ? aPlatformVersion
                                        : DEFAULT_UPDATE_VERSION) + "\" ";
  }
  let buildID = aBuildID ? aBuildID : "20080811053724";
  // XXXrstrong - not specifying a detailsURL will cause a leak due to bug 470244
//   let detailsURL = aDetailsURL ? "detailsURL=\"" + aDetailsURL + "\" " : "";
  let detailsURL = "detailsURL=\"" +
                   (aDetailsURL ? aDetailsURL
                                : "http://test_details/") + "\" ";
  let billboardURL = aBillboardURL ? "billboardURL=\"" +
                                     aBillboardURL + "\" "
                                   : "";
  let licenseURL = aLicenseURL ? "licenseURL=\"" + aLicenseURL + "\" " : "";
  let showPrompt = aShowPrompt ? "showPrompt=\"" + aShowPrompt + "\" " : "";
  let showNeverForVersion = aShowNeverForVersion ? "showNeverForVersion=\"" +
                                                   aShowNeverForVersion + "\" "
                                                 : "";
  let showSurvey = aShowSurvey ? "showSurvey=\"" + aShowSurvey + "\" " : "";
  let custom1 = aCustom1 ? aCustom1 + " " : "";
  let custom2 = aCustom2 ? aCustom2 + " " : "";
  return "  <update type=\"" + type + "\" " +
                   "name=\"" + name + "\" " +
                    displayVersion +
                    version +
                    appVersion +
                    extensionVersion +
                    platformVersion +
                    detailsURL +
                    billboardURL +
                    licenseURL +
                    showPrompt +
                    showNeverForVersion +
                    showSurvey +
                    custom1 +
                    custom2 +
                   "buildID=\"" + buildID + "\"";
}

/**
 * Constructs a string representing a patch element for an update xml file.
 *
 * @param  aType (optional)
 *         The patch's type which should be complete or partial.
 *         If not specified it will default to 'complete'.
 * @param  aURL (optional)
 *         The patch's url to the mar file.
 *         If not specified it will default to the value of:
 *         URL_HOST + URL_PATH + "/" + FILE_SIMPLE_MAR
 * @param  aHashFunction (optional)
 *         The patch's hash function used to verify the mar file.
 *         If not specified it will default to 'MD5'.
 * @param  aHashValue (optional)
 *         The patch's hash value used to verify the mar file.
 *         If not specified it will default to the value of MD5_HASH_SIMPLE_MAR
 *         which is the MD5 hash value for the file specified by FILE_SIMPLE_MAR.
 * @param  aSize (optional)
 *         The patch's file size for the mar file.
 *         If not specified it will default to the file size for FILE_SIMPLE_MAR
 *         specified by SIZE_SIMPLE_MAR.
 * @return The string representing a patch element for an update xml file.
 */
function getPatchString(aType, aURL, aHashFunction, aHashValue, aSize) {
  let type = aType ? aType : "complete";
  let url = aURL ? aURL : URL_HOST + URL_PATH + "/" + FILE_SIMPLE_MAR;
  let hashFunction = aHashFunction ? aHashFunction : "MD5";
  let hashValue = aHashValue ? aHashValue : MD5_HASH_SIMPLE_MAR;
  let size = aSize ? aSize : SIZE_SIMPLE_MAR;
  return "    <patch type=\"" + type + "\" " +
                     "URL=\"" + url + "\" " +
                     "hashFunction=\"" + hashFunction + "\" " +
                     "hashValue=\"" + hashValue + "\" " +
                     "size=\"" + size + "\"";
}
