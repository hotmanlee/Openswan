#include "../lp12-parentR2/parentR2_head.c"
#include "seam_host_jamesjohnson.c"
#include "seam_x509.c"

#define TESTNAME "deleteChildSA-invalid-fromR2"

static void init_local_interface(void)
{
    init_jamesjohnson_interface();
}

static void init_fake_secrets(void)
{
    osw_load_preshared_secrets(&pluto_secrets
			       , TRUE
			       , "../samples/jj.secrets"
			       , NULL, NULL);
}

static void init_loaded(void)
{   /* nothing */ }

#define WANT_THIS_DBG DBG_EMITTING|DBG_PARSING|DBG_CONTROL|DBG_CONTROLMORE|DBG_CRYPT|DBG_PRIVATE

void enable_debugging(void)
{
    base_debugging = WANT_THIS_DBG;
    reset_debugging();
}

void enable_debugging_on_sa(int num)
{
    struct state *st;
    lset_t to_enable = WANT_THIS_DBG;
    st = state_with_serialno(num);
    if(st != NULL) {
        passert(st->st_connection != NULL);
        st->st_connection->extra_debugging = to_enable;
    }
}

void recv_pcap_packet2_and_delete(u_char *user
                      , const struct pcap_pkthdr *h
                      , const u_char *bytes)
{
    static int call_counter = 0;
    struct state *st, *pst;
    struct pcr_kenonce *kn = &crypto_req->pcr_d.kn;

    call_counter++;
    DBG_log("%s() call %d: enter", __func__, call_counter);

    enable_debugging();
    enable_debugging_on_sa(1);
    enable_debugging_on_sa(2);

    recv_pcap_packet2(user, h, bytes);

    /* confirm that SA 1 is in R2 */
    pst = state_with_serialno(1);
    DBG_log("%s() call %d: #%lu st_state=%u\n",
	    __func__, call_counter, pst->st_serialno, pst->st_state);
    passert(pst != NULL);
    passert(pst->st_state == STATE_PARENT_R2);

    /* find st involved */
    st = state_with_serialno(2);
    DBG_log("%s() call %d: #%lu st_state=%u\n",
	    __func__, call_counter, st->st_serialno, st->st_state);
    passert(st != NULL);
    passert(st->st_state == STATE_CHILD_C1_KEYED);

    DBG_log("%s() call %d: exit", __func__, call_counter);
}

void recv_pcap_packet3_end_delete(u_char *user
                      , const struct pcap_pkthdr *h
                      , const u_char *bytes)
{
    static int call_counter = 0;
    struct state *st, *pst;
    struct pcr_kenonce *kn = &crypto_req->pcr_d.kn;

    call_counter++;
    DBG_log("%s() call %d: enter", __func__, call_counter);

    enable_debugging();
    enable_debugging_on_sa(1);
    enable_debugging_on_sa(2);

    /* confirm that SA 1 is in R2 */
    pst = state_with_serialno(1);
    DBG_log("%s() call %d: #%lu st_state=%u\n",
	    __func__, call_counter, pst->st_serialno, pst->st_state);
    passert(pst != NULL);
    passert(pst->st_state == STATE_PARENT_R2);

    /* find st involved */
    st = state_with_serialno(2);
    DBG_log("%s() call %d: #%lu st_state=%u\n",
	    __func__, call_counter, st->st_serialno, st->st_state);
    passert(st != NULL);
    passert(st->st_state == STATE_CHILD_C1_KEYED);

    /* pretend that we initiated the EXPIRE, actually done in lp82, and we
     * are going to process the response to it */
    st->st_state = STATE_DELETING;

    recv_pcap_packet2(user, h, bytes);

    DBG_log("%s() call %d: exit", __func__, call_counter);
}


#ifndef PCAP_INPUT_COUNT
#define PCAP_INPUT_COUNT 3
recv_pcap recv_inputs[PCAP_INPUT_COUNT]={
    recv_pcap_packet,
    recv_pcap_packet2_and_delete,
    recv_pcap_packet3_end_delete,
};
#endif

#include "../lp12-parentR2/parentR2_main.c"

 /*
 * Local Variables:
 * c-style: pluto
 * c-basic-offset: 4
 * compile-command: "make check"
 * End:
 */
