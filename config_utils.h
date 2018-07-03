#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"


#ifndef FJT_CONFIG_UTILS_H
#define FJT_CONFIG_UTILS_H

const char * add_cookie_domain(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_url_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_hkword_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_insert_css(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_base_url(cmd_parms *cmd, void *dummy, const char *ss);
const char *redirect_tip_string(cmd_parms *cmd, void *dummy, const char *ss);
const char *redirect_tip_url(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_surl_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char* use_table(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_js_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_http_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_frame_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *add_extra_data(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_redirect_absolute(cmd_parms *cmd, void *dummy, int flag);
const char *set_extra_data_ignore_js(cmd_parms *cmd, void *dummy, int flag);
const char *set_should_expand_js(cmd_parms *cmd, void *dummy, int flag);
const char *set_from_encode(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_to_encode(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_api_out_encode(cmd_parms *cmd, void *dummy, const char *ss);

const char *set_change_url_in_server(cmd_parms *cmd, void *dummy, int flag);
const char *set_convert_word(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_unicode(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_utf8(cmd_parms *cmd, void *dummy, int flag);
const char *postdata_reset_url(cmd_parms *cmd, void *dummy, int flag);
const char *auto_use_unicode(cmd_parms *cmd, void *dummy, int flag);
const char *force_output_unicode(cmd_parms *cmd, void *dummy, int flag);
const char *process_word(cmd_parms *cmd, void *dummy, int flag);
const char *unconvert_whole_url(cmd_parms *cmd, void *dummy, int flag);
const char *nomime_noconvert(cmd_parms *cmd, void *dummy, int flag);
const char *ignore_urlencode(cmd_parms *cmd, void *dummy, int flag);
const char *notset_contenttype_charset(cmd_parms *cmd, void *dummy, int flag);
const char *mark_utf8url(cmd_parms *cmd, void *dummy, const char *ss);
const char *need_unescape(cmd_parms *cmd, void *dummy, const char *ss);
const char *output_escape_char(cmd_parms *cmd, void *dummy, const char *ss);
const char *notconvert_getpostdata(cmd_parms *cmd, void *dummy, int flag);
const char *notmodify_contenttype(cmd_parms *cmd, void *dummy, int flag);
const char *ignore_urlprefix(cmd_parms *cmd, void *dummy, int flag);
const char *use_unicode_table(cmd_parms *cmd, void *dummy, int flag);
const char *output_same_encode(cmd_parms *cmd, void *dummy, int flag);
const char *keep_utf8_encode(cmd_parms *cmd, void *dummy, int flag);
const char *treat_script_as_html(cmd_parms *cmd, void *dummy, int flag);
const char *ignore_postdata_encode(cmd_parms *cmd, void *dummy, int flag);
const char *notdetect_attachment(cmd_parms *cmd, void *dummy, int flag);
const char *post_rawdata_asutf8(cmd_parms *cmd, void *dummy, int flag);
const char *unescape_postdata(cmd_parms *cmd, void *dummy, int flag);
const char *handle_oldurl_as_seekphoto(cmd_parms *cmd, void *dummy, int flag);
const char *postdata_is_utf8(cmd_parms *cmd, void *dummy, int flag);
const char *escape_utf8_char(cmd_parms *cmd, void *dummy, int flag);
const char *force_output_utf8(cmd_parms *cmd, void *dummy, int flag);
const char *force_post_utf8(cmd_parms *cmd, void *dummy, int flag);
const char *not_convert_page(cmd_parms *cmd, void *dummy, int flag);
const char *use_orig_proxy(cmd_parms *cmd, void *dummy, int flag);
const char *not_process_percent25(cmd_parms *cmd, void *dummy, int flag);
const char *redirect_tip(cmd_parms *cmd, void *dummy, int flag);
const char *process_picture(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_UnconvertOutput(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_NotChangeTextboxUrl(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_NotSetCookie(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_ResumeLogs(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_DetectUTF8(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_DetectUTF8BOM(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_is_ChangedUrlEncode(cmd_parms *cmd, void *dummy, int flag);
const char *set_is_nocache(cmd_parms *cmd, void *dummy, int flag);
const char *set_in_convert_unicode(cmd_parms *cmd, void *dummy, int flag);
const char *set_seekphoto(cmd_parms *cmd, void *dummy, int flag);
const char *set_getclientip(cmd_parms *cmd, void *dummy, int flag);
const char *set_has_hkword(cmd_parms *cmd, void *dummy, int flag);
const char *set_in_should_convert(cmd_parms *cmd, void *dummy, int flag);
const char *set_in_should_convert_word(cmd_parms *cmd, void *dummy, int flag);
const char *set_replace_http_in_text(cmd_parms *cmd, void *dummy, int flag);
const char *set_add_trailing_slash(cmd_parms *cmd, void *dummy, int flag);

const char *set_convert_domain(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_addr_domain(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_host_addr_port(cmd_parms *parms, void *dummy, int flag);
const char *set_page_size(cmd_parms *parms, void *struct_ptr, const char *arg);
const char *set_enable_convert_api(cmd_parms *parms, void *dummy, int flag);
const char *set_binfile_ext(cmd_parms *parms, void *struct_ptr, const char *arg);
const char *set_htmlfile_ext(cmd_parms *parms, void *struct_ptr, const char *arg);
const char *set_keepurlsuffix_ext(cmd_parms *parms, void *struct_ptr, const char *arg);
const char *add_surl_prefix(cmd_parms *cmd, void *dummy, const char *ss);
const char *use_table(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_change_chinese_level(cmd_parms *parms, void *dummy, const char *arg);
const char *redirect_tip_time(cmd_parms *parms, void *dummy, const char *arg);
const char *force_redirect_tip(cmd_parms *cmd, void *dummy, int flag);

const char *set_extra_cookie(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_unconvertsymbol(cmd_parms *cmd, void *dummy,const char *ss);
const char *set_setcontenttype(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_native_domain(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_cookie_path_mode(cmd_parms *cmd, void *dummy, const char *ss);

const char *set_url_map(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_url_mapNE(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_url_modify(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_url_mapJS(cmd_parms *cmd, void *dummy, const char *f, const char *r);
const char *set_process_script_level(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_script_change_chinese_level(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_change_script_by_default(cmd_parms *cmd, void *dummy, const char *ss);
const char *set_value_change_chinese(cmd_parms *cmd, void *dummy, const char *ss);

const char *send_url_utf(cmd_parms *cmd, void *dummy, int flag);
const char *not_replace_url(cmd_parms *cmd, void *dummy, int flag);
const char *convert_cookie(cmd_parms *cmd, void *dummy, int flag);
const char *convert_ContentDisposition(cmd_parms *cmd, void *dummy, int flag);
const char *merge_cookie(cmd_parms *cmd, void *dummy, int flag);
const char *add_url_prefix_to_parameter(cmd_parms *cmd, void *dummy, int flag);
const char *force_convert_page(cmd_parms *cmd, void *dummy, int flag);

const char *set_not_licensed_page(cmd_parms *cmd, void *dummy, const char *ss);

int match_domain(char *domain, apr_table_t *allowed_domains);
const char *set_not_convert_404(cmd_parms *cmd, void *dummy, int flag);

const char *set_is_api(cmd_parms *cmd, void *dummy,int flag);
#endif