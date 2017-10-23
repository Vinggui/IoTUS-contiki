/**
 * \defgroup core of the edytee stack
 *
 * This core is responsible for managing the whole network operation.
 *
 * @{
 */


/*
 * edytee-core-manager.c
 *
 *  Created on: Aug 10, 2017
 *      Author: Vinicius
 */


/*---------------------------------------------------------------------------*/
/**
 * \brief      Receives and process every raw message from the sink node.
 * \param msg  The raw message received by the sink node.
 * \return     Informs the sink that the message was received or not.
 * \retval 0   Failed to decode message.
 * \retval 1   Message decoded and received successfully.
 *
 *             Every new raw message arriving to the sink node is
 *             transmitted to this core, so that it can decide what to
 *             do with the network. Also, it manages do tree network and
 *             takes some decisions.
 *
 */
int edytee_core_manager_push_msg(const char *msg)
{

}



/*---------------------------------------------------------------------------*/

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
