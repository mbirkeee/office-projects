�
 TBATCHIMPORTFORM 0�  TPF0TBatchImportFormBatchImportFormLeftCTopeBorderIconsbiSystemMenu BorderStylebsDialogCaptionBatch Document ImportClientHeight�ClientWidth)Color	clBtnFaceFont.CharsetDEFAULT_CHARSET
Font.ColorclWindowTextFont.Height�	Font.NameMS Sans Serif
Font.Style OldCreateOrderPositionpoScreenCenterOnClose	FormCloseOnPaint	FormPaintPixelsPerInch`
TextHeight TBevelBevel1Left Top Width)Height�Visible  TRadioGroupPhnRadioGroupLeftTopXWidth� HeightICaptionPHN LocationItems.StringsContentsFile Name/RecordBoth TabOrder  TButtonCancelButtonLeft�ToppWidthIHeightCaptionCancelTabOrderOnClickCancelButtonClick  TButtonStartButtonLeftTopWidth� HeightCaptionStart ImportTabOrder OnClickStartButtonClick  	TListView
ListViewInLeftTop8Width� HeightColumnsCaptionDocuments To ImportMaxWidth� Width�   IconOptions.WrapTextReadOnly	SortTypestTextTabOrder	ViewStylevsReport
OnDblClickListViewInDblClick  TPageControlPageControl1LeftTopWidthHeightQ
ActivePageSuccessTabSheet	TabHeightTabIndex TabOrder 	TTabSheetSuccessTabSheetCaptionImport Succeeded 	TListViewListViewSucceededLeftTopWidth� HeightColumnsCaptionDocuments Successfully ImportedWidth�   ReadOnly	SortTypestTextTabOrder 	ViewStylevsReport
OnDblClickListViewSucceededDblClick   	TTabSheetFailedTabSheetCaptionImport Failed
ImageIndex 	TListViewListViewFailedLeftTopWidth� HeightColumnsCaptionDocuments Not ImportedMaxWidth� Width�   TabOrder 	ViewStylevsReport
OnDblClickListViewFailedDblClick    	TGroupBox	GroupBox1LeftTopWidth� HeightICaptionDocument TypeTabOrder 	TComboBoxDocumentTypeComboBoxLeftTopWidthpHeight
ItemHeightTabOrder TextDocumentTypeComboBoxOnChangeDocumentTypeComboBoxChange   	TGroupBox	GroupBox2Left�TopWidth� HeightICaption	Fail WhenTabOrder 	TCheckBoxFailProviderCheckBoxLeftTop0Width� HeightCaptionProvider not determinedTabOrder  	TCheckBoxFailDateCheckBoxLeftTop WidthyHeightCaptionDate not determinedTabOrder   	TCheckBoxFailPhnCheckBoxLeftTopWidth� HeightCaptionPHN not determinedTabOrder   TRadioGroupPromptRadioGroupLeft�TopXWidth� HeightICaption	Prompt OnItems.StringsAllFailedNone TabOrder  TRadioGroupProviderRadioGroupLeft� TopWidth� HeightICaptionProviderItems.Strings	DetermineSelectedNone TabOrder  TRadioGroupDateRadioGroupLeft� TopXWidth� HeightICaptionDateItems.Strings	DetermineSelectedNone TabOrder	  TEditDateEditLeft� TopwWidth� HeightTabOrder
TextDateEditOnMouseDownDateEditMouseDown  	TGroupBox	GroupBox3LeftTop� WidthyHeightYCaptionDocument LocationsTabOrder TLabelLabel1LeftTopWidthAHeight	AlignmenttaRightJustifyAutoSizeCaptionImport From:LayouttlCenter  TLabelLabel2LeftTop)Width9Height	AlignmenttaRightJustifyAutoSizeCaption
Failed To:LayouttlCenter  TEdit
ImportEditLeftPTopWidth!HeightTabOrder Text
ImportEditOnMouseDownImportEditMouseDown  TEdit
FailedEditLeftPTop(Width!HeightTabOrderText
FailedEditOnMouseDownFailedEditMouseDown  	TCheckBoxMoveFailedCheckBoxLeftPTop@Width� HeightCaption)Move Failed Documents to Failed DirectoryTabOrderOnClickMoveFailedCheckBoxClick  TBitBtnImportDirSelectButtonLeft^TopWidthHeightTabOrderOnClickImportDirSelectButtonClick
Glyph.Data
^   Z   BMZ       >   (                                    ��� �   �   �   �   �       �     TBitBtnFailedDirSelectButtonLeft^Top*WidthHeightTabOrderOnClickFailedDirSelectButtonClick
Glyph.Data
^   Z   BMZ       >   (                                    ��� �   �   �   �   �       �      TBitBtnDateSelectButtonLeftfTopyWidthHeightTabOrderOnClickDateSelectButtonClick
Glyph.Data
^   Z   BMZ       >   (                                    ��� �   �   �   �   �       �     	TComboBoxProviderComboBoxLeft� Top'Width� Height
ItemHeightTabOrderTextProviderComboBoxOnChangeProviderComboBoxChange  	TGroupBox	GroupBox4Left�Top� Width� HeightYCaptionDocument CountsTabOrder TLabelLabel3LeftTopWidth4Height	AlignmenttaRightJustifyAutoSizeCaptionTotal:  TLabelLabel4LeftTop0Width4Height	AlignmenttaRightJustifyAutoSizeCaption	Imported:  TLabelLabel5LeftTop@Width4Height	AlignmenttaRightJustifyAutoSizeCaptionFailed:  TLabelLabel6LeftTop Width4Height	AlignmenttaRightJustifyAutoSizeCaption
Remaining:  TLabelTotalCountLabelLeftHTopWidth9Height	AlignmenttaRightJustifyAutoSizeCaptionTotalCountLabel  TLabelImportedCountLabelLeftHTop0Width9Height	AlignmenttaRightJustifyAutoSizeCaptionImportedCountLabel  TLabelFailedCountLabelLeftHTop@Width9Height	AlignmenttaRightJustifyAutoSizeCaptionFailedCountLabel  TLabelRemainingCountLabelLeftHTop Width9Height	AlignmenttaRightJustifyAutoSizeCaptionRemainingCountLabel    