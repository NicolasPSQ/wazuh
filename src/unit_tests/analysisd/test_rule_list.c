/*
 * Copyright (C) 2015-2020, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../../headers/shared.h"
#include "../../analysisd/eventinfo.h"
#include "../../analysisd/cdb/cdb.h"
#include "../../analysisd/analysisd.h"

void os_count_rules(RuleNode *node, int *num_rules);
void os_remove_rulenode(RuleNode *node, RuleInfo **rules, int *pos, int *max_size);
void os_remove_ruleinfo(RuleInfo *ruleinfo);
void os_remove_rules_list(RuleNode *node);
int OS_AddChild(RuleInfo *read_rule, RuleNode **r_node, OSList* log_msg);

/* setup/teardown */



/* wraps */

void __wrap_OSMatch_FreePattern(OSMatch *reg) {
    return;
}

void __wrap_OSRegex_FreePattern(OSRegex *reg) {
    return;
}

void __wrap_os_remove_cdbrules(ListRule **l_rule) {
    os_free(*l_rule);
    return;
}

void __wrap__os_analysisd_add_logmsg(OSList * list, int level, int line, const char * func,
                                    const char * file, char * msg, ...) {
    char formatted_msg[OS_MAXSTR];
    va_list args;

    va_start(args, msg);
    vsnprintf(formatted_msg, OS_MAXSTR, msg, args);
    va_end(args);

    check_expected(level);
    check_expected_ptr(list);
    check_expected(formatted_msg);
}


/* tests */

/* os_count_rules */
void test_os_count_rules_no_child(void **state)
{
    RuleNode *node;
    os_calloc(1,sizeof(RuleNode), node);

    int num_rules = 0;

    os_count_rules(node, &num_rules);

    os_free(node);

}

void test_os_count_rules_child(void **state)
{
    RuleNode *node;
    os_calloc(1, sizeof(RuleNode), node);
    os_calloc(1, sizeof(OSDecoderNode), node->child);

    int num_rules = 0;

    os_count_rules(node, &num_rules);

    os_free(node->child);
    os_free(node);

}

/* os_remove_rulenode */
void test_os_remove_rulenode_no_child(void **state)
{
    int pos = 0;
    int max_size = 2;

    RuleNode * node;
    os_calloc(1, sizeof(RuleNode), node);
    os_calloc(1, sizeof(RuleInfo), node->ruleinfo);
    node->ruleinfo->internal_saving = false;

    RuleInfo **rules_info;
    os_calloc(1, sizeof(OSDecoderInfo *), rules_info);

    int num_decoders = 0;

    os_remove_rulenode(node, rules_info, &pos, &max_size);

    os_free(rules_info[0]);
    os_free(rules_info);

}

void test_os_remove_rulenode_child(void **state)
{
    int pos = 0;
    int max_size = 2;

    RuleNode * node;
    os_calloc(1, sizeof(RuleNode), node);
    os_calloc(1, sizeof(RuleNode), node->child);
    os_calloc(1, sizeof(RuleInfo), node->ruleinfo);
    os_calloc(1, sizeof(RuleInfo), node->child->ruleinfo);
    node->ruleinfo->internal_saving = false;
    node->child->ruleinfo->internal_saving = false;

    RuleInfo **rules_info;
    os_calloc(2, sizeof(RuleInfo *), rules_info);

    int num_decoders = 0;

    os_remove_rulenode(node, rules_info, &pos, &max_size);

    os_free(rules_info[0]);
    os_free(rules_info[1]);
    os_free(rules_info);

}

/* os_remove_ruleinfo */
void test_os_remove_ruleinfo_NULL(void **state)
{
    RuleInfo *ruleinfo = NULL;

    os_remove_ruleinfo(ruleinfo);

}

void test_os_remove_ruleinfo_OK(void **state)
{
    RuleInfo *ruleinfo;
    os_calloc(1, sizeof(RuleInfo), ruleinfo);

    os_calloc(2, sizeof(char*), ruleinfo->ignore_fields);
    os_strdup("test_ignore_fields", ruleinfo->ignore_fields[0]);

    os_calloc(2, sizeof(char*), ruleinfo->ckignore_fields);
    os_strdup("test_ckignore_felds", ruleinfo->ckignore_fields[0]);

    os_calloc(2, sizeof(os_ip*), ruleinfo->srcip);
    os_calloc(1, sizeof(os_ip), ruleinfo->srcip[0]);
    os_strdup("0.0.0.0", ruleinfo->srcip[0]->ip);

    os_calloc(2, sizeof(os_ip*), ruleinfo->dstip);
    os_calloc(1, sizeof(os_ip), ruleinfo->dstip[0]);
    os_strdup("0.0.0.0", ruleinfo->dstip[0]->ip);

    os_calloc(2, sizeof(FieldInfo*), ruleinfo->fields);
    os_calloc(1, sizeof(FieldInfo), ruleinfo->fields[0]);
    os_strdup("test_name", ruleinfo->fields[0]->name);
    os_calloc(1, sizeof(OSRegex), ruleinfo->fields[0]->regex);

    os_calloc(1, sizeof(RuleInfoDetail), ruleinfo->info_details);

    os_calloc(2, sizeof(active_response*), ruleinfo->ar);
    os_calloc(1, sizeof(active_response), ruleinfo->ar[0]);
    os_strdup("test_ar_name", ruleinfo->ar[0]->name);
    os_strdup("test_ar_command", ruleinfo->ar[0]->command);
    os_strdup("test_ar_agent_id", ruleinfo->ar[0]->agent_id);
    os_strdup("test_ar_rules_id", ruleinfo->ar[0]->rules_id);
    os_strdup("testtest_ar_rules_group", ruleinfo->ar[0]->rules_group);
    os_calloc(1, sizeof(ar_command), ruleinfo->ar[0]->ar_cmd);
    os_strdup("test_ar_command_name", ruleinfo->ar[0]->ar_cmd->name);
    os_strdup("test_ar_command_executable", ruleinfo->ar[0]->ar_cmd->executable);
    os_strdup("test_ar_command_extra_args", ruleinfo->ar[0]->ar_cmd->extra_args);

    os_calloc(1, sizeof(ListRule), ruleinfo->lists);

    os_calloc(2, sizeof(char*), ruleinfo->same_fields);
    os_strdup("test_same_fields", ruleinfo->same_fields[0]);

    os_calloc(2, sizeof(char*), ruleinfo->not_same_fields);
    os_strdup("test_not_same_fields", ruleinfo->not_same_fields[0]);

    os_calloc(2, sizeof(char*), ruleinfo->mitre_id);
    os_strdup("test_mitre_id", ruleinfo->mitre_id[0]);

    os_calloc(1, sizeof(OSMatch), ruleinfo->match);

    os_calloc(1, sizeof(OSRegex), ruleinfo->regex);

    os_calloc(1, sizeof(OSMatch), ruleinfo->dstgeoip);

    os_calloc(1, sizeof(OSMatch), ruleinfo->srcport);

    os_calloc(1, sizeof(OSMatch), ruleinfo->dstport);

    os_calloc(1, sizeof(OSMatch), ruleinfo->user);

    os_calloc(1, sizeof(OSMatch), ruleinfo->url);

    os_calloc(1, sizeof(OSMatch), ruleinfo->id);

    os_calloc(1, sizeof(OSMatch), ruleinfo->status);

    os_calloc(1, sizeof(OSMatch), ruleinfo->hostname);

    os_calloc(1, sizeof(OSMatch), ruleinfo->program_name);

    os_calloc(1, sizeof(OSMatch), ruleinfo->data);

    os_calloc(1, sizeof(OSMatch), ruleinfo->extra_data);

    os_calloc(1, sizeof(OSMatch), ruleinfo->location);

    os_calloc(1, sizeof(OSMatch), ruleinfo->system_name);

    os_calloc(1, sizeof(OSMatch), ruleinfo->protocol);

    os_calloc(1, sizeof(OSRegex), ruleinfo->if_matched_regex);

    os_calloc(1, sizeof(OSMatch), ruleinfo->if_matched_group);

    os_remove_ruleinfo(ruleinfo);

}

/* os_remove_rules_list */
void test_os_remove_rules_list_OK(void **state)
{
    RuleNode *node;
    os_calloc(1,sizeof(RuleNode), node);

    os_calloc(1, sizeof(RuleInfo), node->ruleinfo);

    os_calloc(2, sizeof(char*), node->ruleinfo->ignore_fields);
    os_strdup("test_ignore_fields", node->ruleinfo->ignore_fields[0]);

    os_calloc(2, sizeof(char*), node->ruleinfo->ckignore_fields);
    os_strdup("test_ckignore_felds", node->ruleinfo->ckignore_fields[0]);

    os_calloc(2, sizeof(os_ip*), node->ruleinfo->srcip);
    os_calloc(1, sizeof(os_ip), node->ruleinfo->srcip[0]);
    os_strdup("0.0.0.0", node->ruleinfo->srcip[0]->ip);

    os_calloc(2, sizeof(os_ip*), node->ruleinfo->dstip);
    os_calloc(1, sizeof(os_ip), node->ruleinfo->dstip[0]);
    os_strdup("0.0.0.0", node->ruleinfo->dstip[0]->ip);

    os_calloc(2, sizeof(FieldInfo*), node->ruleinfo->fields);
    os_calloc(1, sizeof(FieldInfo), node->ruleinfo->fields[0]);
    os_strdup("test_name", node->ruleinfo->fields[0]->name);
    os_calloc(1, sizeof(OSRegex), node->ruleinfo->fields[0]->regex);

    os_calloc(1, sizeof(RuleInfoDetail), node->ruleinfo->info_details);

    os_calloc(2, sizeof(active_response*), node->ruleinfo->ar);
    os_calloc(1, sizeof(active_response), node->ruleinfo->ar[0]);
    os_strdup("test_ar_name", node->ruleinfo->ar[0]->name);
    os_strdup("test_ar_command", node->ruleinfo->ar[0]->command);
    os_strdup("test_ar_agent_id", node->ruleinfo->ar[0]->agent_id);
    os_strdup("test_ar_rules_id", node->ruleinfo->ar[0]->rules_id);
    os_strdup("testtest_ar_rules_group", node->ruleinfo->ar[0]->rules_group);
    os_calloc(1, sizeof(ar_command), node->ruleinfo->ar[0]->ar_cmd);
    os_strdup("test_ar_command_name", node->ruleinfo->ar[0]->ar_cmd->name);
    os_strdup("test_ar_command_executable", node->ruleinfo->ar[0]->ar_cmd->executable);
    os_strdup("test_ar_command_extra_args", node->ruleinfo->ar[0]->ar_cmd->extra_args);

    os_calloc(1, sizeof(ListRule), node->ruleinfo->lists);

    os_calloc(2, sizeof(char*), node->ruleinfo->same_fields);
    os_strdup("test_same_fields", node->ruleinfo->same_fields[0]);

    os_calloc(2, sizeof(char*), node->ruleinfo->not_same_fields);
    os_strdup("test_same_fields", node->ruleinfo->not_same_fields[0]);

    os_calloc(2, sizeof(char*), node->ruleinfo->mitre_id);
    os_strdup("test_mitre_id", node->ruleinfo->mitre_id[0]);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->match);

    os_calloc(1, sizeof(OSRegex), node->ruleinfo->regex);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->dstgeoip);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->srcport);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->dstport);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->user);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->url);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->id);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->status);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->hostname);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->program_name);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->data);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->extra_data);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->location);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->system_name);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->protocol);

    os_calloc(1, sizeof(OSRegex), node->ruleinfo->if_matched_regex);

    os_calloc(1, sizeof(OSMatch), node->ruleinfo->if_matched_group);

    os_remove_rules_list(node);

}

// Test OS_AddChild
void test_OS_AddChild_inconsistent_rule(void ** state) {
    RuleInfo * read_rule = NULL;
    RuleNode * r_node = NULL;
    OSList list_msg = {0};
    const int expect_value = 1;
    int retval;

    expect_value(__wrap__os_analysisd_add_logmsg, level, LOGLEVEL_ERROR);
    expect_value(__wrap__os_analysisd_add_logmsg, list, &list_msg);
    expect_string(__wrap__os_analysisd_add_logmsg, formatted_msg,
                  "rules_list: Passing a NULL rule. Inconsistent state");

    retval = OS_AddChild(read_rule, &r_node, &list_msg);

    assert_int_equal(retval, expect_value);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        // Tests os_count_rules
        cmocka_unit_test(test_os_count_rules_no_child),
        cmocka_unit_test(test_os_count_rules_child),
        // Tests os_remove_rulenode
        cmocka_unit_test(test_os_remove_rulenode_no_child),
        cmocka_unit_test(test_os_remove_rulenode_child),
        // Tests os_remove_ruleinfo
        cmocka_unit_test(test_os_remove_ruleinfo_NULL),
        cmocka_unit_test(test_os_remove_ruleinfo_OK),
        // Tests os_remove_rules_list
        cmocka_unit_test(test_os_remove_rules_list_OK),
        // Tests OS_AddChild
        cmocka_unit_test(test_OS_AddChild_inconsistent_rule),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
