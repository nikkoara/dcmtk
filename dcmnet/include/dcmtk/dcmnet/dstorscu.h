/*
 *
 *  Copyright (C) 2011, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmnet
 *
 *  Author:  Joerg Riesmeier
 *
 *  Purpose: DICOM Storage Service Class User (SCU)
 *
 *  Last Update:      $Author: joergr $
 *  Update Date:      $Date: 2011-10-19 13:26:44 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DSTORSCU_H
#define DSTORSCU_H

#include "dcmtk/config/osconfig.h"  /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/scu.h"       /* for base class DcmSCU */


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Interface class for a Storage Service Class User (SCU).
 *  This class supports C-STORE messages as an SCU.  In a first step, the SOP instances to be
 *  sent are added to a transfer list.  In a second step, the association negotiation takes
 *  place where the required presentation contexts are proposed, i.e. it is checked which SOP
 *  classes and transfer syntaxes are needed for transferring the SOP instances.  Finally, the
 *  SOP instances are sent to the SCP (if possible).
 *  \note
 *    \li The current implementation does not sort the transfer list according to the SOP
 *        Class UID and Transfer Syntax UID of the SOP instances and, therefore, might propose
 *        more presentation contexts than required for the transfer of all SOP instances.  A
 *        simple optimization that is performed internally is to check whether the current SOP
 *        instance can be sent using a presentation context that has previously been added for
 *        another SOP instance (of the same kind).  This approach also makes sure that studies
 *        and series are not mixed up, assuming that they have been added to the transfer list
 *        in the "correct" order.
 *    \li Another limitation of the current implementation is the handling of the "Default
 *        Transfer Syntax" in case of compression.  According to the DICOM standard, the
 *        default transfer syntax for "Lossless JPEG Compression", "Lossy JPEG Compression"
 *        and so on has to be proposed in at least one presentation context for the particular
 *        SOP class.  This is not (yet) implemented since the re-encoding of compressed
 *        datasets is not supported.  Nevertheless, depending on the options used, the default
 *        transfer syntax for the uncompressed case is always proposed (if possible).
 */
class DcmStorageSCU
  : public DcmSCU
{

  public:

    /** dataset decompression modes
     */
    enum E_DecompressionMode
    {
        /// never decompress datasets
        DM_never,
        /// decompress lossless only
        DM_losslessOnly,
        /// default value: decompress lossless only
        DM_default = DM_losslessOnly,
        /// decompress both lossy and lossless
        DM_lossyAndLossless
    };

    /** dataset handling modes
     */
    enum E_HandlingMode
    {
        /// do nothing with the dataset
        HM_doNothing,
        /// compact the dataset after it has been sent
        HM_compactAfterSend,
        /// delete the dataset after it has been sent
        HM_deleteAfterSend,
        /// delete the dataset after it is has been removed from the transfer list
        HM_deleteAfterRemove
    };

    /** default constructor
     */
    DcmStorageSCU();

    /** destructor
     */
    virtual ~DcmStorageSCU();

    /** clear the internal member variables
     */
    virtual void clear();

    /** get value of the association counter.  This counter is initialized with 0 (that means
     *  no association exists) and increased by 1 for every new association.  The counter is
     *  only reset when clear() is called.
     *  @return value of the association counter
     */
    unsigned long getAssociationCounter() const;

    /** get number of SOP instances stored in the transfer list
     *  @return number of SOP instances stored in the transfer list
     */
    size_t getNumberOfSOPInstances() const;

    /** get number of SOP instances that are to be sent (i.e.\ that are not yet sent).  Please
     *  note that internally the transfer list is iterated to determine this number, so
     *  depending on the size of the list it might take some time.
     *  @return number of SOP instances that are to be sent
     */
    size_t getNumberOfSOPInstancesToBeSent() const;

    /** get mode that specifies whether or not compressed datasets are decompressed if needed,
     *  i.e.\ whether the transfer syntax of the dataset is changed for network transmission.
     *  @return decompression mode. See definition of E_DecompressionMode for possible values.
     */
    E_DecompressionMode getDecompressionMode() const;

    /** get mode that specifies whether to halt if unsuccessful store encountered or whether
     *  to continue with the next SOP instance.
     *  @return mode indicating whether to halt on unsuccessful store or not
     */
    OFBool getHaltOnUnsuccessfulStoreMode() const;

    /** get mode that specifies whether to propose presentation contexts that do not contain
     *  the default transfer syntax although it is needed, which might result in a violation
     *  of the DICOM standard.
     *  @return mode indicating whether illegal proposals are allowed or not
     */
    OFBool getAllowIllegalProposalMode() const;

    /** set mode that specifies whether or not compressed datasets are decompressed if needed,
     *  i.e.\ whether the transfer syntax of the dataset is changed for network transmission.
     *  @param  decompressionMode  decompression mode. See definition of E_DecompressionMode
     *                             for both possible values and the default value.
     */
    void setDecompressionMode(const E_DecompressionMode decompressionMode);

    /** set mode that specifies whether to halt if unsuccessful store encountered or whether
     *  to continue with the next SOP instance.
     *  @param  haltMode  mode indicating whether to halt or not (default: OFTrue, i.e.\ halt)
     */
    void setHaltOnUnsuccessfulStoreMode(const OFBool haltMode);

    /** set mode that specifies whether to propose presentation contexts that do not contain
     *  the default transfer syntax, although it is needed, which might result in a violation
     *  of the DICOM standard.  For example, if a lossless compressed SOP instance is to be
     *  sent, there should be at least one presentation context for this SOP class that also
     *  proposes the default transfer syntax (Implicit VR Little Endian).
     *  @param  allowMode  mode indicating whether illegal proposals are allowed or not
     *                     (default: OFTrue, i.e.\ allowed)
     */
    void setAllowIllegalProposalMode(const OFBool allowMode);

    /** reset the sent status for all SOP instances in the transfer list.  This alllows for
     *  sending the same SOP instances again - on the same or a different association.
     *  @param  sameAssociation  flag indicating whether the same assocation will be used for
     *                           the transfer as last time.  If a different association will
     *                           be used, also the presentation context IDs are set to 0
     *                           (undefined), which means that addPresentationContexts() has
     *                           to be called again.  Please make sure that all dataset
     *                           pointers in the transfer list are still valid, i.e. the
     *                           datasets have not been deleted.
     */
    void resetSentStatus(const OFBool sameAssociation = OFFalse);

    /** remove all SOP instances from the transfer list.  If an entry contains a reference to
     *  a DICOM dataset, this dataset is deleted if the handling mode HM_deleteAfterRemove was
     *  used to add it to the transfer list.
     */
    void removeAllSOPInstances();

    /** remove a particular SOP instance from the transfer list.  If the corresponding entry
     *  contains a reference to a DICOM dataset, this dataset is deleted if the handling mode
     *  HM_deleteAfterRemove was used to add it to the transfer list.
     *  @param  sopClassUID     SOP Class UID of the SOP instance to be removed
     *  @param  sopInstanceUID  SOP Instance UID of the SOP instance to be removed
     *  @param  allOccurrences  flag specifying whether to delete all occurrences of the
     *                          SOP instance if it has been added to the list multiple times.
     *                          If OFFalse, only the first occurrence is removed.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition removeSOPInstance(const OFString &sopClassUID,
                                  const OFString &sopInstanceUID,
                                  const OFBool allOccurrences = OFTrue);

    /** add a SOP instance stored as a DICOM file to the list of instances to be transferred.
     *  Before adding the SOP instance to the list, it is checked for validity and conformance
     *  to the DICOM standard (see checkSOPInstance() for details).  However, duplicate
     *  instances are not recognized, i.e. they are added to the list and later on transferred
     *  to the storage SCP when calling sendSOPInstances().
     *  @param  filename     name of the DICOM file that contains the SOP instance to be sent
     *  @param  readMode     read mode passed to the DcmFileFormat::loadFile() method.  If
     *                       ERM_fileOnly, only the file meta information header is loaded,
     *                       i.e. the behavior is identical to using ERM_metaOnly.
     *  @param  checkValues  flag indicating whether to check the UID values for validity and
     *                       conformance.  If OFFalse, only empty values are rejected.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition addDicomFile(const OFString &filename,
                             const E_FileReadMode readMode = ERM_fileOnly,
                             const OFBool checkValues = OFTrue);

    /** add a SOP instance from a given DICOM dataset to the list of instances to be
     *  transferred. Before adding the SOP instance to the list, it is checked for validity
     *  and conformance to the DICOM standard (see checkSOPInstance() for details).  However,
     *  duplicate instances are not recognized, i.e. they are added to the list and later on
     *  transferred to the storage SCP when calling sendSOPInstances().
     *  @param  dataset       DICOM dataset that contains the SOP instance to be sent
     *  @param  datasetXfer   transfer syntax of the dataset (determined automatically if
     *                        unknown, which is also the default)
     *  @param  handlingMode  mode specifying what to do with the dataset if no longer needed.
     *                        HM_xxxAfterSend has no effect if the C-STORE request could not
     *                        be sent.
     *                        Please do not add the same dataset multiple times with a mode of
     *                        HM_deleteAfterXXX, since it will result in deleting the same
     *                        object multiple times!
     *  @param  checkValues   flag indicating whether to check the UID values for validity and
     *                        conformance.  If OFFalse, only empty values are rejected.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition addDataset(DcmDataset *dataset,
                           const E_TransferSyntax datasetXfer = EXS_Unknown,
                           const E_HandlingMode handlingMode = HM_compactAfterSend,
                           const OFBool checkValues = OFTrue);

    /** add presentation contexts for all SOP instances in the transfer list, which were not
     *  yet sent (either successfully or unsuccessfully).  Initially, the internal list of
     *  presentation contexts is cleared.  Then, the transfer list is iterated and a new
     *  presentation context is added for each SOP instance that cannot be sent using any of
     *  the previously added presentation contexts.  If the maximum of 128 presentation
     *  contexts, which can be negotiated during a single association, is reached, this method
     *  returns and any subsequent call adds the next bunch of presentation contexts needed.
     *  @return status, EC_Normal if successful, an error code otherwise.  If no presentation
     *     contexts have been added, NET_EC_NoPresentationContextsDefined is returned.  This
     *     code can, therefore, be used to check that all SOP instances from the transfer list
     *     have been negotiated and sent in previous calls.
     */
    OFCondition addPresentationContexts();

    /** negotiate association by using presentation contexts and parameters as defined by
     *  earlier method calls.  If negotiation fails, there is no need to close the
     *  association or to do anything else with this class.
     *  In addition to the implementation inherited from the base class DcmSCU, this method
     *  also handles the case that none of the proposed presentation contexts was accepted.
     *  And, it also increases the association counter by 1 for each new association
     *  (including the ones that are not successful).
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition negotiateAssociation();

    /** send SOP instances to be transferred to the specified peer.  After the presentation
     *  contexts for the current association have been added and negotiated, the SOP instances
     *  that are to be sent on this association can be transferred with this method.  Unless
     *  the corresponding mode is disabled using setHaltOnUnsuccessfulStoreMode(), the
     *  transfer is stopped on the first SOP instance that could not be "stored" successfully.
     *  Please note, however, that the DIMSE status is not checked for this purpose, i.e. the
     *  transfer is never stopped when the DIMSE status is different from 0x0000 (success).
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition sendSOPInstances();

    /** release the current association by sending an A-RELEASE request to the SCP.  Please
     *  note that this release only applies to associations that have been created by calling
     *  DcmSCU::initNetwork() and DcmSCU::negotiateAssociation().
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition releaseAssociation();

    /** get some status information on the overall sending process.  This text can for example
     *  be output to the logger (on the level at the user's option).
     *  @param  summary  reference to a string in which the summary is stored
     */
    void getStatusSummary(OFString &summary) const;

    /** create a text file with a detailed report on the transfer of DICOM SOP instances.  In
     *  addition to a general header and some status information at the end (as generated by
     *  getStatusSummary()), a couple of basic details on each SOP instance in the transfer
     *  list are written to the file:
     *  - Consecutive number
     *  - Name of the DICOM file
     *  - SOP Instance UID
     *  - SOP Class UID (and associated name)
     *  - Transfer Syntax UID (and associated name)
     *  - Number of the association (that was used to transfer the instance)
     *  - Presentation Context ID (that was used to transfer the instance)
     *  - DIMSE Status (if instance was sent) or reason why it was not sent
     *  @param  filename  name of the text file in which the report is stored
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition createReportFile(const OFString &filename) const;


  protected:

    /** internal class/struct for a single transfer entry
     */
    struct TransferEntry
    {
        /** constructor. Initializes member variables with reasonable values.
         *  @param  filename           filename of the SOP instance to be transferred
         *  @param  readMode           mode to read the given SOP instance from file
         *  @param  sopClassUID        SOP Class UID of the SOP instance to be transferred
         *  @param  sopInstanceUID     SOP Instance UID of the SOP instance to be transferred
         *  @param  transferSyntaxUID  Transfer Syntax UID of the SOP instance to be
         *                             transferred
         */
        TransferEntry(const OFString &filename,
                      const E_FileReadMode readMode,
                      const OFString &sopClassUID,
                      const OFString &sopInstanceUID,
                      const OFString &transferSyntaxUID);

        /** constructor. Initializes member variables with reasonable values.
         *  @param  dataset            pointer to the dataset of the SOP instance to be
         *                             transferred
         *  @param  handlingMode       mode specifying what to do with the dataset if no
         *                             longer needed
         *  @param  sopClassUID        SOP Class UID of the SOP instance to be transferred
         *  @param  sopInstanceUID     SOP Instance UID of the SOP instance to be transferred
         *  @param  transferSyntaxUID  Transfer Syntax UID of the SOP instance to be
         *                             transferred
         */
        TransferEntry(DcmDataset *dataset,
                      const E_HandlingMode handlingMode,
                      const OFString &sopClassUID,
                      const OFString &sopInstanceUID,
                      const OFString &transferSyntaxUID);

        /** destructor
         */
        ~TransferEntry();

        /// filename of the SOP instance to be transferred (if no 'Dataset' given)
        const OFString Filename;
        /// read mode that should be used to read the given SOP instance from file
        const E_FileReadMode FileReadMode;
        /// dataset of the SOP instance to be transferred (if no 'Filename' given)
        DcmDataset *Dataset;
        /// handling mode specifying what to do with the dataset if no longer needed
        const E_HandlingMode DatasetHandlingMode;
        /// SOP Class UID of the SOP instance to be transferred
        const OFString SOPClassUID;
        /// SOP Instance UID of the SOP instance to be transferred
        const OFString SOPInstanceUID;
        /// Transfer Syntax UID of the SOP instance to be transferred
        const OFString TransferSyntaxUID;
        /// transfer syntax that was used to send this SOP instance
        E_TransferSyntax NetworkTransferSyntax;
        /// flag indicating whether the SOP instance is uncompressed, i.e.\ uses any of the
        /// three uncompressed transfer syntaxes
        OFBool Uncompressed;
        /// association number that was used to send this SOP instance (0 = not sent)
        unsigned long AssociationNumber;
        /// presentation context ID to be used for sending this SOP instance
        T_ASC_PresentationContextID PresentationContextID;
        /// flag indicating whether the C-STORE request has been sent (OFTrue = sent)
        OFBool RequestSent;
        /// DIMSE status of the C-STORE response (0x0000 = success)
        Uint16 ResponseStatusCode;

      private:

        /** initialize further member variables
         */
        void Init();

        // private undefined copy constructor
        TransferEntry(const TransferEntry &);

        // private undefined assignment operator
        TransferEntry &operator=(const TransferEntry &);
    };

    // --- static methods ---

    /** get SOP Class UID, SOP Instance UID and Transfer Syntax UID from a DICOM file.  The
     *  first two UID values are either copied from the meta-header (preferred) or from the
     *  dataset.  The latter is either copied from the meta-header (preferred) or determined
     *  automatically (if possible).
     *  @param  filename           name of the DICOM file from which the SOP Class UID and SOP
     *                             Instance UID values are retrieved
     *  @param  sopClassUID        variable in which the value of the SOP Class UID is stored
     *  @param  sopInstanceUID     variable in which the value of the SOP Instance UID is
     *                             stored
     *  @param  transferSyntaxUID  variable in which the value of the Transfer Syntax UID is
     *                             stored
     *  @param  readMode           read mode passed to the DcmFileFormat::loadFile() method.
     *                             If ERM_fileOnly, only the file meta information header is
     *                             loaded, i.e. the behavior is identical to using
     *                             ERM_metaOnly.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    static OFCondition getSOPInstanceFromFile(const OFString &filename,
                                              OFString &sopClassUID,
                                              OFString &sopInstanceUID,
                                              OFString &transferSyntaxUID,
                                              const E_FileReadMode readMode);

    /** get SOP Class UID, SOP Instance UID and Transfer Syntax UID from a DICOM dataset.
     *  The first two UID values are directly copied from the dataset.  The latter is either
     *  taken from the parameter 'datasetXfer' or, if it is unknown, determined automatically
     *  from the dataset (if possible).
     *  @param  dataset            DICOM dataset from which the SOP Class UID and SOP Instance
     *                             UID values are retrieved
     *  @param  datasetXfer        transfer syntax of the dataset (if known, otherwise it is
     *                             determined automatically)
     *  @param  sopClassUID        variable in which the value of the SOP Class UID is stored
     *  @param  sopInstanceUID     variable in which the value of the SOP Instance UID is
     *                             stored
     *  @param  transferSyntaxUID  variable in which the value of the Transfer Syntax UID is
     *                             stored
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    static OFCondition getSOPInstanceFromDataset(DcmDataset *dataset,
                                                 const E_TransferSyntax datasetXfer,
                                                 OFString &sopClassUID,
                                                 OFString &sopInstanceUID,
                                                 OFString &transferSyntaxUID);

    /** check given SOP Class UID, SOP Instance UID and Transfer Syntax UID for validity and
     *  conformance to the DICOM standard.  For all UID values, the compliance with the
     *  requirements of the value representation "Unique Identifier" (UI) and value
     *  multiplicity is checked.  For the SOP Class UID, it is also checked whether it is a
     *  known or at least a possible storage SOP class (either a standard or a private one).
     *  For the Transfer Syntax UID, it is checked whether it is known and generally
     *  supported.  Further checks will be performed when the list of presentation contexts is
     *  created for the association negotiation.
     *  @param  sopClassUID        value of the SOP Class UID to be checked
     *  @param  sopInstanceUID     value of the SOP Instance UID to be checked
     *  @param  transferSyntaxUID  value of the Transfer Syntax UID to be checked
     *  @param  checkValues        flag indicating whether to check the UID values for
     *                             validity and conformance.  If OFFalse, only empty values
     *                             are rejected.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    static OFCondition checkSOPInstance(const OFString &sopClassUID,
                                        const OFString &sopInstanceUID,
                                        const OFString &transferSyntaxUID,
                                        const OFBool checkValues);


  private:

    /// association counter
    unsigned long AssociationCounter;
    /// presentation context counter
    unsigned long PresentationContextCounter;
    /// decompression mode, i.e.\ whether a dataset is decompressed for transmission
    E_DecompressionMode DecompressionMode;
    /// flag indicating whether to halt on unsuccessful
    OFBool HaltOnUnsuccessfulStoreMode;
    /// flag indicating whether to allow illegal proposals
    OFBool AllowIllegalProposalMode;
    /// list of SOP instances to be transferred
    OFList<TransferEntry *> TransferList;
    /// iterator pointing to the current entry in the list of SOP instances to be transferred
    OFListIterator(TransferEntry *) CurrentTransferEntry;

    // private undefined copy constructor
    DcmStorageSCU(const DcmStorageSCU &);

    // private undefined assignment operator
    DcmStorageSCU &operator=(const DcmStorageSCU &);
};

#endif // DCMSTORSCU_H


/*
 * CVS Log
 * $Log: dstorscu.h,v $
 * Revision 1.3  2011-10-19 13:26:44  joergr
 * Fixed some typos and other minor issues regarding the comments.
 *
 * Revision 1.2  2011-10-06 14:16:08  joergr
 * Now also SOP instances from DICOM datasets can be added to the transfer list.
 * This allows for sending datasets created or received in memory.
 *
 * Revision 1.1  2011-10-05 13:33:34  joergr
 * Added easy-to-use interface class for a Storage Service Class User (SCU).
 *
 *
 */