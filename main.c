/* 
 * This program enables to dissect the
 * IMSI number (International Mobile Subscriber Identity)
 * Autor : MELLO NGUIE Jean Prince
 * Creation date : 06/09/2022
 * Update date : 06/09/2022
 * Revision : v1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define GET_NETWORK_NODE_OF_CURRENT_NODE(cur_node)              (cur_node)->prev->prev->prev->prev
#define GET_COUNTRY_NODE_OF_CURRENT_NODE(cur_node)              (cur_node)->prev->prev
#define GET_ISO_NODE_OF_CURRENT_NODE(cur_node)                  (cur_node)->next->next		
#define GET_COUNTRY_CODE_NODE_OF_CURRENT_NODE(cur_node)         (cur_node)->next->next->next->next
#define GET_MNC_NODE_OF_CURRENT_NODE(cur_node)                  (cur_node)->next->next->next->next->next->next


#define MCC_MNC_TABLE	"mcc-mnc-table.xml"

#define IMSI_MIN_LEN	14	/* Min size of IMSI */
#define IMSI_MAX_LEN	15	/* Max size of IMSI */
#define MCC_LEN	        3	/* Size of Mobile Country Code */
#define MNC_MIN_LEN	2	/* Min size of Mobile Network Code */
#define MNC_MAX_LEN	3	/* Max size of Mobile Network Code */
#define MSIN_MIN_LEN	9	/* Min size of Mobile Subscription Identification Number */
#define MSIN_MAX_LEN	10	/* Max size of Mobile Subscription Identification Number */


static void
print_imsi_header(void)
{
    fprintf(stdout, "MCC;MNC;MSIN;NETWORK;COUNTRY;REGION\n");
}

static void
print_imsi_info(const char *mcc, const char *mnc, const char *msin,
		const char *network_name, const char *country_name, const char *mcc_region_name)
{
    fprintf(stdout, "%.*s;%.*s;%.*s;%.*s;%.*s;%.*s\n", (int)strlen(mcc), mcc,
                                                        (int)strlen(mnc), mnc,
                                                        (int)strlen(msin), msin,
                                                        (int)strlen(network_name), network_name,
                                                        (int)strlen(country_name), country_name,
                                                        (int)strlen(mcc_region_name), mcc_region_name);
}

static int
find_extra_info(xmlDoc *doc, xmlNode *a_node, const char *mcc_str,
                char *mnc_str, xmlChar **network_name, xmlChar **country_name, int *found)
{
    xmlNode *cur_node = NULL;
    xmlChar *local_mcc_value = NULL;
    xmlChar *local_mnc_value = NULL;
    xmlChar *network_str = NULL;
    xmlChar *country_str = NULL;

    int nb_found = 0;
    int nb_print = 0;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            /* printf("node type: Element, name: %s\n", cur_node->name); */
            /* 1 - We look first for the MCC attribute */
            if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"mcc")))
            {
                local_mcc_value = xmlNodeListGetString(doc, (cur_node)->children, 1);

                /* 2 - We compare the local mcc value and the current mcc value  */
                if ((!xmlStrcmp(local_mcc_value, (const xmlChar *)mcc_str)))
                {	
                    /* 3 - We look for the MNC attribute related to the MCC */
                    if ((!xmlStrcmp(GET_MNC_NODE_OF_CURRENT_NODE(cur_node)->name, (const xmlChar *)"mnc")))
                    {
                        local_mnc_value = xmlNodeListGetString(doc, GET_MNC_NODE_OF_CURRENT_NODE(cur_node)->children, 1);
                        if ((!xmlStrcmp(local_mcc_value, (const xmlChar *)mcc_str)) &&
                             !xmlStrcmp(local_mnc_value, (const xmlChar *)mnc_str))
                        {
                            if (nb_found == 0)
                            {
                                nb_found = 1;
                                *found = nb_found;
                                /*printf("MNC: %s\n", local_mnc_value);*/
                                nb_found++;

                                /* 4 - We look for the network at which the MCC belongs */
                                if ((!xmlStrcmp(GET_NETWORK_NODE_OF_CURRENT_NODE(cur_node)->name, (const xmlChar *)"network")))
                                {
                                    /* We print the network only once */
                                    if (nb_print >= 0)
                                    {
                                        network_str = xmlNodeListGetString(doc, GET_NETWORK_NODE_OF_CURRENT_NODE(cur_node)->children, 1);
                                        *network_name = network_str;
                                        /*printf("NETWORK: %s\n", network_str);*/
                                        nb_print++;
                                    }
                                }

                                /* 5 - We look for the country at which the MCC belongs */
                                if ((!xmlStrcmp(GET_COUNTRY_NODE_OF_CURRENT_NODE(cur_node)->name, (const xmlChar *)"country")))
                                {
                                    if (nb_print >= 0)
                                    {
                                        country_str = xmlNodeListGetString(doc, GET_COUNTRY_NODE_OF_CURRENT_NODE(cur_node)->children, 1);
                                        *country_name = country_str;
                                        /*printf("COUNTRY: %s\n", country_str);*/
                                        nb_print++;
                                    }
                                }
                                xmlFree(local_mnc_value);
                                xmlFree(local_mcc_value);
                                local_mnc_value = NULL;
                                local_mcc_value = NULL;
                                return 0;
                            }
                        }
                        xmlFree(local_mnc_value);
                        xmlFree(local_mcc_value);
                        local_mnc_value = NULL;
                        local_mcc_value = NULL;
                    }
                    if (local_mnc_value != NULL) {
                        xmlFree(local_mnc_value);
                        local_mnc_value = NULL;
                    }
                    find_extra_info(doc, GET_MNC_NODE_OF_CURRENT_NODE(cur_node)->children,
                                         mcc_str,
                                         mnc_str,
                                         network_name,
                                         country_name,
                                         found);
                }
            }
            if (local_mcc_value != NULL) {
                xmlFree(local_mcc_value);
                local_mcc_value = NULL;
            }
            find_extra_info(doc, cur_node->children,
                                 mcc_str,
                                 mnc_str,
                                 network_name,
                                 country_name,
                                 found);
        }
    }

    return 0;
}

/* This function identifies the geographic region
 * based on the first MCC digit.
 * Accoding ITU standard, the range of MCC are as following :
 * 200 - 299 => Europe
 * 300 - 399 => North America
 * 400 - 499 => Asia
 * 500 - 599 => Oceania
 * 600 - 699 => Africa
 * 700 - 799 => South America 
 * 900 - 999 => Worldwide
 * */
char *get_geographic_region_of_mcc_digits(const char *mcc)
{
    switch ((mcc[0] - '0'))
    {
        case 0:
            return "Test networks";
        case 2:
            return "Europe";
        case 3:
            return "North America and the Caribbean";
        case 4:
            return "Asia and the Middle East";
        case 5:
            return "Australia and Oceania";
        case 6:
            return "Africa";
        case 7:
            return "South and Central America";
        case 9:
            return "Worldwide";
        default:
        return "Unknown geographic region !";
    }
}

int mem_alloc_and_init(char **mem_space, int mem_space_len)
{
    *mem_space = (char *)malloc(mem_space_len + 1);
    if (*mem_space == NULL)
    {
        fprintf(stderr, "Memory allocation failed !\n");
        return -1;
    }

    memset(*mem_space, 0, mem_space_len + 1);

    return 0;
}

int get_mcc_from_imsi(const char *imsi, char **mcc_str, int mcc_str_len)
{
    int i;
    int ret;

    ret = mem_alloc_and_init(mcc_str, mcc_str_len);
    if (ret < 0)
    {
        return ret;
    }

    for (i = 0; i < mcc_str_len; i++)
    {
        *(*mcc_str + i) = imsi[i];
    }
    *(*mcc_str + i) = '\0';

    return 0;
}

int check_imsi_digits(const char *imsi, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (!isdigit(imsi[i]))
        {
            fprintf(stderr, "'%c' at the index %d is not a digit !\n", imsi[i], i);
            return -1;
        }
    }

    return 0;
}

int check_imsi_len(const char *imsi)
{
    int len;
    len = (int)strlen(imsi);

    if (len < IMSI_MIN_LEN || len > IMSI_MAX_LEN)
    {
        fprintf(stderr, "Wrong IMSI length (%u) !\nValues should be 14 or 15 digits long.\n", len);
        return -1;
    }
	
    return len;
}

int get_first_n_bytes(const char *imsi, char **mnc_tmp_str, int start, int mnc_tmp_str_len)
{
    int i;
    int j;
    int ret;

    ret = mem_alloc_and_init(mnc_tmp_str, mnc_tmp_str_len);
    if (ret < 0)
    {
        return ret;
    }

    for (i = 0, j = start; i < mnc_tmp_str_len && j < (j + mnc_tmp_str_len); i++, j++)
    {
        *(*mnc_tmp_str + i) = imsi[j];
    }
    *(*mnc_tmp_str + i) = '\0';

    return 0;
}

int main(int argc, char *argv[])
{
    int len;
    int ret;
    int i;
    char *mcc_str = NULL;
    char *mcc_region_name = NULL;
    char *mnc_tmp_str = NULL;
    char *mnc_str = NULL;
    int mnc_str_len = 0;
    char *msin_str = NULL;

    int mnc_found = 0;
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlChar *country_name = NULL;
    xmlChar *network_name = NULL;

    /**
     ** This initialize the library and check potential ABI mismatches
     ** between the version it was compiled for and the actual shared
     ** library used.
     **/
    LIBXML_TEST_VERSION
   
    /* Check the number of arguments */
    if (argc != 2)
    {
        fprintf(stderr, "%s 310150123456789\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Check IMSI length */
    len = check_imsi_len(argv[1]);
    if (len < 0)
    {
        return EXIT_FAILURE;
    }

    /* Parse the file and get the DOM */
    doc = xmlReadFile(MCC_MNC_TABLE, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "error: could not parse file %s\n", MCC_MNC_TABLE);
    }

    /* Check if IMSI has got digits only */
    ret = check_imsi_digits(argv[1], len);
    if (ret < 0)
    {
        return EXIT_FAILURE;
    }

    /* Get the MCC */
    ret = get_mcc_from_imsi(argv[1], &mcc_str, MCC_LEN);
    if (ret < 0)
    {
        return EXIT_FAILURE;
    }
    /*fprintf(stdout, "MCC: %.*s\n", MCC_LEN, mcc_str);*/

    /* Get the geographic region name 
     * at which the MCC belongs */
    mcc_region_name = get_geographic_region_of_mcc_digits(mcc_str);
    /*fprintf(stdout, "REGION: %.*s\n", (int)strlen(mcc_region_name), mcc_region_name);*/

    /* Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    /* We try to find the MNC with 2 or 3 bytes */
    for (i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            /* Get the first two bytes for MNC checking */
            ret = get_first_n_bytes(argv[1], &mnc_tmp_str, 3, MNC_MIN_LEN);
            if (ret < 0)
            {
                return EXIT_FAILURE;
            }
        }

        if (i == 1)
        {
            /* Get the first three bytes for MNC checking */
            ret = get_first_n_bytes(argv[1], &mnc_tmp_str, 3, MNC_MAX_LEN);
            if (ret < 0)
            {
                return EXIT_FAILURE;
            }
        }

        find_extra_info(doc, root_element, mcc_str,
                                           mnc_tmp_str,
                                           &network_name,
                                           &country_name,
                                           &mnc_found);
        if (mnc_found)
        {
            /* We have found MNC */
            mnc_str = mnc_tmp_str;
        }
        else
        {
            if (i == 0) {
                fprintf(stderr, "MNC '%s' not found for MCC '%s' !\n", mnc_tmp_str, mcc_str);
                continue;
            } else {
                fprintf(stderr, "MNC '%s' not found for MCC '%s' !\n", mnc_tmp_str, mcc_str);
                return EXIT_FAILURE;
            }
        }
    
        /* Get the lenght of MNC */
        mnc_str_len = (int)strlen(mnc_str);

        /* Get the remaining bytes after MNC : MSIN */
        if (mnc_str_len == 3) {
            ret = get_first_n_bytes(argv[1], &msin_str, 6, MSIN_MIN_LEN);
            if (ret < 0)
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            ret = get_first_n_bytes(argv[1], &msin_str, 5, MSIN_MAX_LEN);
            if (ret < 0)
            {
                return EXIT_FAILURE;
            }
        }

        if (i == 0)
        {
            print_imsi_header();
            print_imsi_info(mcc_str, mnc_str, msin_str,
                                            (const char *)network_name,
                                            (const char *)country_name,
                                            mcc_region_name);
        }
        else
        {
            if (network_name != NULL &&
                country_name != NULL) {
                print_imsi_info(mcc_str, mnc_str, msin_str,
                                                (const char *)network_name,
                                                (const char *)country_name,
                                                mcc_region_name);
            }
        }

        xmlFree(network_name);
        xmlFree(country_name);
        network_name = NULL;
        country_name = NULL;

        free(msin_str);
        free(mnc_tmp_str);
    }

    xmlFreeDoc(doc);

    /*
     ** Free the global variables that may
     ** have been allocated by the parser.
     **/
    xmlCleanupParser();

    free(mcc_str);

    return EXIT_SUCCESS;
}
