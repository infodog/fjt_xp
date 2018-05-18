#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "config.h"
#include "config_utils.h"
#include "mod_fjt.h"
#include "license.h"

const char *add_cookie_domain(cmd_parms *cmd, void *dummy, const char *ss){
    config *conf = (config*)dummy;
    if(conf){
        strcpy(conf->m_pcCookieDomain ,ss);
    }
    return NULL;
}

const char *add_url_prefix(cmd_parms *cmd, void *dummy, const char *ss){

    config *conf = (config*)dummy;

    if(conf==NULL){
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "add_url_prefix conf==NULL,return");
        return NULL;
    }

    strcpy(conf->m_pcUrlPrefix, ss);
    {
        char *pp = strstr(ss, "://");
        if (pp != NULL) {
            pp += 3;
            if (CheckUrlPrefix(cmd->pool, pp) == 0) {
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "[error] The UrlPrefix (%s) is not licensed.", ss);
                exit(-1);
            }
        }
    }

    {/* // WPF Begin it. */
        const char *info = ss;

        static int i = 0;
        static char str[128] = {'\0'};
        if (i == 0) {
            int n = 60;
            str[i++] = (char)n;
            str[i++] = (char)(n + 13);
            str[i++] = (char)(n + 18);
            str[i++] = (char)(n + 10);
            str[i++] = (char)(n + 19);
            str[i++] = (char)(n + 23);
            str[i++] = (char)(n + 7);
            str[i++] = (char)(n + 5);
            str[i++] = (char)(n + 20);
            str[i++] = (char)(n + 9);
            str[i++] = (char)(n + 2);
            str[i] = '\0';
        }
        if (info != NULL && i > 0) {
            int j = 0;
            while (j < i) {
                if (*(info + j) == '\0') {
                    break;
                }
                else if (*(info + j) == str[j]) {
                    j++;
                }
                else {
                    j = 0;
                    info++;
                }
            }
            if (j == i) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, 0, NULL, "%s,author:zxy@xinshi.net,wpf@xinshi.net",info);
            }
        }
    }/* // WPF End it. */

    return NULL;
}

const char *add_hkword_prefix(cmd_parms *cmd, void *dummy,const char *ss){
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcHkPrefix,ss);

    return NULL;
}

const char *add_insert_css(cmd_parms *cmd, void *dummy, const char *ss){
   config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcInsertCSS, ss);
    return NULL;
}

const char *set_base_url(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcConfBaseUrl, ss);
    return NULL;
}

const char *redirect_tip_string(cmd_parms *cmd, void *dummy,const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strncpy(conf->m_pcRedirectTipString, ss, 4095);
    return NULL;
}

const char *redirect_tip_url(cmd_parms *cmd, void *dummy, const char *ss){
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    strcpy(conf->m_pcRedirectTipURL, ss);
    return NULL;
}

const char *add_surl_prefix(cmd_parms *cmd, void *dummy, const char *ss){

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcSUrlPrefix,ss);
    {
        char *pp = strstr(ss, "://");
        if (pp != NULL) {
            pp += 3;
            if (CheckUrlPrefix(cmd->pool, pp) == 0) {
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "[error] The UrlPrefix (%s) is not licensed.", ss);
                exit(-1);
            }
        }
    }

    {/* // WPF Begin it. */
        char *info = ss;

        static int i = 0;
        static char str[128] = {'\0'};
        if (i == 0) {
            int n = 60;
            str[i++] = (char)n;
            str[i++] = (char)(n + 13);
            str[i++] = (char)(n + 18);
            str[i++] = (char)(n + 10);
            str[i++] = (char)(n + 19);
            str[i++] = (char)(n + 23);
            str[i++] = (char)(n + 7);
            str[i++] = (char)(n + 5);
            str[i++] = (char)(n + 20);
            str[i++] = (char)(n + 9);
            str[i++] = (char)(n + 2);
            str[i] = '\0';
        }
        if (info != NULL && i > 0) {
            int j = 0;
            while (j < i) {
                if (*(info + j) == '\0') {
                    break;
                }
                else if (*(info + j) == str[j]) {
                    j++;
                }
                else {
                    j = 0;
                    info++;
                }
            }
            if (j == i) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, 0, NULL, "%s",info);
            }
        }
    }/* // WPF End it. */
    return NULL;
}



const char *add_js_prefix(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcJsPrefix,ss);
    return NULL;
}

const char *add_http_prefix(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcHTTPPrefix ,ss);
    return NULL;
}

const char *add_frame_prefix(cmd_parms *cmd, void *dummy, const char *ss){
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcFramePrefix, ss);
    return NULL;
}

const char *add_extra_data(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    strcpy(conf->m_pcExtraData, ss);
    return NULL;
}

const char *set_redirect_absolute(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    conf->m_iIsRedirectAbsolute  = flag;
    return NULL;
}

const char *set_extra_data_ignore_js(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iExtraDataIgnoreJs  = flag;
    return NULL;
}

const char *set_should_expand_js(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iShouldExpandJs  = flag;
    return NULL;
}

const char *set_from_encode(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (strnicmp(ss,"GB2312",6)==0)
    {
        conf->m_iFromEncode = ENCODE_GB2312;
    }
    else if (strnicmp(ss,"BIG5",4) == 0)
    {
        conf->m_iFromEncode = ENCODE_BIG5;
    }
    return NULL;
}

const char *set_to_encode(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    if (strnicmp(ss,"GB2312",6)==0)
    {
        conf->m_iToEncode = ENCODE_GB2312;
    }
    else if (strnicmp(ss,"BIG5",4) == 0)
    {
        conf->m_iToEncode = ENCODE_BIG5;
    }

    return NULL;
}

const char *set_change_url_in_server(cmd_parms *cmd, void *dummy, const int flag){
   config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iShouldChangeUrlInServer  = flag;
    return NULL;
}

const char *set_convert_word(cmd_parms *cmd, void *dummy, int flag)
{
   config *conf = (config*)dummy;
   if(!conf){
        return NULL;
   }
   if (cmd->path == NULL)
       return NULL;
   conf->m_iConvertWord = flag;
   return NULL;
}

const char *set_is_unicode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIsUnicode = flag;
    return NULL;
}

const char *set_is_utf8(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIsUTF8 = flag;
    return NULL;
}

const char *postdata_reset_url(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iPostdataResetUrl = flag;
    return NULL;
}

const char *auto_use_unicode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iAutoUseUnicode = flag;
    return NULL;
}

const char *force_output_unicode(cmd_parms *cmd, void *dummy, int flag)
{
   config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iForceOutputUnicode = flag;

    return NULL;
}


const char *process_word(cmd_parms *cmd, void *dummy, int flag)
{
   config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iProcessWord = flag;

    return NULL;
}

const char *unconvert_whole_url(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iUnConvertWholeUrl = flag;

    return NULL;
}

const char *nomime_noconvert(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNoMimeNoConvert = flag;

    return NULL;
}

const char *ignore_urlencode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIgnoreUrlEncode = flag;

    return NULL;
}

const char *notset_contenttype_charset(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotSetContentTypeCharset = flag;

    return NULL;
}

const char *mark_utf8url(cmd_parms *cmd, void *dummy,const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iMarkUTF8Url = atoi(ss);
    return NULL;
}

const char *need_unescape(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    if (cmd->path == NULL) {
        return NULL;
    }
    if (strcmp(ss, "on") == 0) {
        conf->m_iUnescapeChar = 1;
    }
    else {
        conf->m_iUnescapeChar = atoi(ss);
    }

    return NULL;
}

const char *output_escape_char(cmd_parms *cmd, void *dummy,const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL) {
        return NULL;
    }
    if (strcmp(ss, "on") == 0) {
        conf->m_iOutputEscapeChar = 1;
    }
    else {
        conf->m_iOutputEscapeChar = atoi(ss);
    }

    return NULL;
}

const char *notconvert_getpostdata(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotConvertGetPostData = flag;

    return NULL;
}

const char *notmodify_contenttype(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotModifyContentType = flag;

    return NULL;
}

const char *ignore_urlprefix(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIgnoreUrlPrefix = flag;

    return NULL;
}

const char *use_unicode_table(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iUseUnicodeTable = flag;

    return NULL;
}



const char *output_same_encode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iOutputSameEncode = flag;

    return NULL;
}

const char *keep_utf8_encode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iKeepUTF8Encode = flag;

    return NULL;
}

const char *treat_script_as_html(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iTreatScriptAsHtml = flag;

    return NULL;
}

const char *ignore_postdata_encode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIgnorePostDataEncode = flag;

    return NULL;
}

const char *notdetect_attachment(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotDetectAttachment = flag;

    return NULL;
}

const char *post_rawdata_asutf8(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iPostRawDataAsUTF8 = flag;

    return NULL;
}

const char *unescape_postdata(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iUnescapePostData = flag;

    return NULL;
}

const char *handle_oldurl_as_seekphoto(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iHandleOLDURL = flag;

    return NULL;
}

const char *postdata_is_utf8(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iPostDataIsUTF8 = flag;

    return NULL;
}

const char *escape_utf8_char(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iEscapeUTF8Char = flag;

    return NULL;
}

const char *force_output_utf8(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iForceOutputUTF8 = flag;

    return NULL;
}

const char *force_post_utf8(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iForcePostUTF8 = flag;

    return NULL;
}

const char *not_convert_page(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotConvertPage = flag;

    return NULL;
}

const char *use_orig_proxy(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iUseOrigProxy = flag;

    return NULL;
}

const char *not_process_percent25(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotProcessPercent25 = flag;

    return NULL;
}

const char *redirect_tip(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iRedirectTip = flag;

    return NULL;
}

const char *process_picture(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iProcessPicture = flag;
    return NULL;
}

const char *set_is_UnconvertOutput(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iUnconvertOutput = flag;
    return NULL;
}

const char *set_is_NotChangeTextboxUrl(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotChangeTextboxUrl = flag;
    return NULL;
}

const char *set_is_NotSetCookie(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotSetCookie = flag;
    return NULL;
}

const char *set_is_ResumeLogs(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iResumeLogs = flag;
    return NULL;
}

const char *set_is_DetectUTF8(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iDetectUTF8 = flag;
    return NULL;
}

const char *set_is_DetectUTF8BOM(cmd_parms *cmd, void *dummy, const char *ss)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iDetectUTF8BOM = atoi(ss);
    return NULL;
}

const char *set_is_ChangedUrlEncode(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iChangedUrlEncode = flag;
    return NULL;
}

const char *set_is_nocache(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNoCache = flag;
    return NULL;
}

const char *set_in_convert_unicode(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iInConvertUnicode = flag;
    return NULL;
}

const char *set_seekphoto(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iSeekPhoto = flag;
    return NULL;
}

const char *set_getclientip(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iGetClientIP = flag;
    return NULL;
}

const char *set_has_hkword(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iIshkword  = flag;
    return NULL;
}

const char *set_in_should_convert(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iInShouldConvert = flag;
    return NULL;
}

const char *set_in_should_convert_word(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iInConvertWord = flag;
    return NULL;
}

const char *set_replace_http_in_text(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iReplaceHttpInText = flag;
    return NULL;
}

const char *set_add_trailing_slash(cmd_parms *cmd, void *dummy, int flag)
{
     config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iAddTrailingSlash = flag;
    return NULL;
}

const char *set_convert_domain(cmd_parms *cmd, void *dummy, const char *f, const char *r)
{
    server_rec *s = cmd->server;
    svr_config *conf =
            (svr_config *) ap_get_module_config(s->module_config, &fjt_module);

    {/* // WPF Begin it. */
        char *info = f;

        static int i = 0;
        static char str[128] = {'\0'};                                              
        if (i == 0) {
            int n = 60;
            str[i++] = (char)n;
            str[i++] = (char)(n + 13);
            str[i++] = (char)(n + 18);
            str[i++] = (char)(n + 10);
            str[i++] = (char)(n + 19);
            str[i++] = (char)(n + 23);
            str[i++] = (char)(n + 7);
            str[i++] = (char)(n + 5);
            str[i++] = (char)(n + 20);
            str[i++] = (char)(n + 9);
            str[i++] = (char)(n + 2);
            str[i] = '\0';
        }
        if (info != NULL && i > 0) {
            int j = 0;
            while (j < i) {
                if (*(info + j) == '\0') {
                    break;
                }
                else if (*(info + j) == str[j]) {
                    j++;
                }
                else {
                    j = 0;
                    info++;
                }
            }
            if (j == i) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, 0, NULL, "%s",info);
            }
        }
    }/* // WPF End it. */

    if (strstr(r, "exclude") != NULL) {
        ++conf->nExcludeDomain;
        apr_table_set(conf->exclude_domain , f, r);
        return NULL;
    }

    if (strstr(r, "friendly") != NULL) {
        ++conf->nFriendlyDomain;
        apr_table_set(conf->friendly_domain , f, r);
        return NULL;
    }

    if (CheckLicense(conf, cmd->pool, f)) {
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "License is ok!");
        ++conf->nYesDomain;
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
            "setting allowed_domain......");

        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "allowed domains:%s:%s",f,r);
        if(conf->allowed_domain==NULL){
            ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                "conf->allowed_domain is NULL");
        }
        else{
//            printf("set allowed domain allowed_domain=%i,%s=%s\n",conf->allowed_domain,f,r);
            apr_table_set(conf->allowed_domain , f, r);
        }
    }
    else {
        ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                     "[notice] The domain (%s) is not licensed.", f);
         exit(-1);
    }

    return NULL;
}

/* // WPF // */
const char *set_addr_domain(cmd_parms *cmd, void *dummy, const char *f, const char *r)
{

    server_rec *s = cmd->server;
    svr_config *conf =
            (svr_config *) ap_get_module_config(s->module_config, &fjt_module);
    apr_port_t ipstr_port = 0;
    apr_port_t domain_port = 0;
    char *ptr = NULL;
    hosts_addr_domain *addrdomain = NULL;

    ptr = strrchr(f, ':');
    if (ptr != NULL) {
        *ptr = '\0';
        ipstr_port = atoi(ptr + 1);
    }
    ptr = strrchr(r, ':');
    if (ptr != NULL) {
        *ptr = '\0';
        domain_port = atoi(ptr + 1);
    }
    addrdomain = (hosts_addr_domain*) apr_pcalloc(cmd->pool, sizeof(hosts_addr_domain));
    addrdomain->ipstr = apr_pstrdup(cmd->pool, f);
    addrdomain->ipstr_port = ipstr_port;
    addrdomain->domain = apr_pstrdup(cmd->pool, r);
    addrdomain->domain_port = domain_port;
    addrdomain->next = NULL;
    ++conf->nAddrDomain;
    if (conf->addr_domain == NULL) {
        conf->addr_domain = addrdomain;
    }
    else {
        hosts_addr_domain *next = conf->addr_domain;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = addrdomain;
    }

    return NULL;
}

const char *set_host_addr_port(cmd_parms *parms, void *dummy, int flag)
{

    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);

    psf->m_iHostAddrPort = flag;
    psf->m_iHostAddrPort_set = 1;

    return NULL;
}

/* // WPF // */
const char *set_page_size(cmd_parms *parms, void *struct_ptr, const char *arg)
{

    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);

    int val;
    if (sscanf(arg, "%d", &val) != 1)
        return "WebPageMaxSize value must be an integer (kBytes)";

    psf->WebPageMaxSize = val * 1024;
    psf->WebPageMaxSize_set = 1;

    return NULL;
}

const char *set_enable_convert_api(cmd_parms *parms, void *dummy, int flag)
{

    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);

    psf->m_iExportApi = flag;
    psf->m_iExportApi_set = 1;

    return NULL;
}

const char *set_binfile_ext(cmd_parms *parms, void *struct_ptr, const char *arg)
{

    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);
    {
        int val = 0;
        char *pb = arg;
        char tmp[1024];
        while (val < 100) {
            char *p = NULL;

            p = strchr(pb, '|');
            if (p != NULL) {
                memcpy(tmp, pb, p - pb);
                tmp[p - pb] = '\0';
                pb = p + 1;
            }
            else {
                strcpy(tmp, pb);
            }

            psf->m_pcBinaryFileExt[val++] = apr_pstrdup(parms->pool, tmp);
            if (p == NULL) {
                break;
            }
        }
        psf->m_iBinaryFileExt = val;
    }


    return NULL;
}

const char *set_htmlfile_ext(cmd_parms *parms, void *struct_ptr, const char *arg)
{
    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);

    {
        int val = 0;
        char *pb = arg;
        char tmp[1024];
        while (val < 100) {
            char *p = NULL;

            p = strchr(pb, '|');
            if (p != NULL) {
                memcpy(tmp, pb, p - pb);
                tmp[p - pb] = '\0';
                pb = p + 1;
            }
            else {
                strcpy(tmp, pb);
            }

            psf->m_pcHtmlFileExt[val++] = apr_pstrdup(parms->pool, tmp);
            if (p == NULL) {
                break;
            }
        }
        psf->m_iHtmlFileExt = val;
    }
    return NULL;
}

const char *set_keepurlsuffix_ext(cmd_parms *parms, void *struct_ptr, const char *arg)
{

    svr_config *psf =
            ap_get_module_config(parms->server->module_config, &fjt_module);

    {
        int val = 0;
        char *pb = arg;
        char tmp[1024];
        while (val < 100) {
            char *p = NULL;

            p = strchr(pb, '|');
            if (p != NULL) {
                memcpy(tmp, pb, p - pb);
                tmp[p - pb] = '\0';
                pb = p + 1;
            }
            else {
                strcpy(tmp, pb);
            }

            psf->m_pcKeepUrlSuffix[val++] = apr_pstrdup(parms->pool, tmp);
            if (p == NULL) {
                break;
            }
        }
        psf->m_iKeepUrlSuffix = val;
    }

    return NULL;
}




const char* use_table(cmd_parms *cmd, void *dummy, const char *ss)
{

    config *conf = (config*)dummy;

    if(!conf){
        return NULL;
    }
    if (conf->m_pUseTableFile == NULL) {
        conf->m_pUseTableFile = apr_table_make(cmd->pool, 10);
    }
    apr_table_set(conf->m_pUseTableFile, ss, ss);

    return NULL;
}

/* // WPF ADD SOURCE CODE */
const char*set_change_chinese_level(cmd_parms *parms, void *dummy, const char *arg)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    conf->m_iChangeChineseLevel = atoi(arg);

    return NULL;
}

const char*redirect_tip_time(cmd_parms *parms, void *dummy, const char *arg)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    conf->m_iRedirectTipTime = atoi(arg);

    return NULL;
}

const char *force_redirect_tip(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iForceRedirectTip = flag;

    return NULL;
}

const char *set_extra_cookie(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    strcpy(conf->m_pcExtraCookie , ss);

    return NULL;
}

const char *set_unconvertsymbol(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    strcpy(conf->m_pcUnconvertSymbol , ss);

    return NULL;
}

const char *set_setcontenttype(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    strcpy(conf->m_pcSetContentType , ss);

    return NULL;
}

const char *set_native_domain(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_pcNativeDomain = ss;

    return NULL;
}

const char *set_cookie_path_mode(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iCookiePathMode = atoi(ss);
    return NULL;
}

const char *set_url_map(cmd_parms *cmd, void *dummy,const char *f, const char *r)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    apr_table_set(conf->m_pUrlMap,f,r);

    {
        char *pp = strstr(r, "://");
        if (pp != NULL) {
            pp += 3;
            if (CheckUrlPrefix(cmd->pool, pp) == 0) {
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "[error] The UrlPrefix (%s) is not licensed.", r);
                exit(-1);
            }
        }
    }

    return NULL;
}

const char *set_url_mapNE(cmd_parms *cmd, void *dummy, const char *f, const char *r)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    apr_table_set(conf->m_pUrlMapNE,f,r);

    {
        char *pp = strstr(r, "://");
        if (pp != NULL) {
            pp += 3;
            if (CheckUrlPrefix(cmd->pool, pp) == 0) {
                ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL, "[error] The UrlPrefix (%s) is not licensed.", r);
                exit(-1);
            }
        }
    }

    return NULL;
}

const char *set_url_modify(cmd_parms *cmd, void *dummy, const char *f, const char *r)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    apr_table_set(conf->m_pUrlModify,f,r);

    return NULL;
}

const char *set_url_mapJS(cmd_parms *cmd, void *dummy, const char *f, const char *r)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }

    apr_table_set(conf->m_pUrlMapJS,f,r);

    return NULL;
}

const char *set_process_script_level(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iProcessScriptLevel = atoi(ss);
    return NULL;
}

const char *set_script_change_chinese_level(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iScriptChangeChineseLevel = atoi(ss);
    return NULL;
}

const char *set_change_script_by_default(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iChangeScriptByDefault = atoi(ss);
    return NULL;
}

const char *set_value_change_chinese(cmd_parms *cmd, void *dummy, const char *ss)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;

    conf->m_iValueChangeChineseLevel = atoi(ss);
    return NULL;
}

const char *force_convert_page(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iForceConvertPage = flag;

    return NULL;
}

const char *send_url_utf(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iSendURLsAsUTF8 = flag;

    return NULL;
}

const char *not_replace_url(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iNotReplaceUrl = flag;

    return NULL;
}

const char *convert_cookie(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iConvertCookie = flag;

    return NULL;
}

const char *convert_ContentDisposition(cmd_parms *cmd, void *dummy, int flag)
{
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iConvertContentDisposition = flag;
    return NULL;
}

const char *merge_cookie(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iMergeCookie = flag;

    return NULL;
}

const char *add_url_prefix_to_parameter(cmd_parms *cmd, void *dummy, int flag)
{

    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_iAddUrlPrefixToParameter = flag;


    return NULL;
}

int match_domain(char *domain, apr_table_t *allowed_domains){
    array_header *reqhdrs_arr;
    table_entry *reqhdrs;
    int i;

//    printf("check_domain_of_license %i\n",allowed_domains);

    reqhdrs_arr = (array_header *) apr_table_elts(allowed_domains);
    if (!reqhdrs_arr) {
        return 0;
    }
    reqhdrs = (table_entry *) reqhdrs_arr->elts;
    int lenDomain = strlen(domain);

    for (i=0; i < reqhdrs_arr->nelts ; i++)
    {
        char *allowedDomain, *toUrl;
        allowedDomain = reqhdrs[i].key;
        int lenAllowedDomain = strlen(allowedDomain);
        if (lenDomain >= lenAllowedDomain && strnicmp(domain+(lenDomain-lenAllowedDomain), allowedDomain, lenAllowedDomain)==0)
        {
           return 1;
        }
    }
    return 0;
}



const char *set_not_licensed_page(cmd_parms *cmd, void *dummy, const char *ss){
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    if (cmd->path == NULL)
        return NULL;
    conf->m_pcNotLicensedPage = apr_pstrdup(cmd->pool,ss);
    return NULL;
}

const char *set_not_convert_404(cmd_parms *cmd, void *dummy, int flag){
    config *conf = (config*)dummy;
    if(!conf){
        return NULL;
    }
    conf->m_iNotConvert404  = flag;
    return NULL;
}

