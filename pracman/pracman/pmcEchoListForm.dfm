object EchoListForm: TEchoListForm
  Left = 55
  Top = 283
  Width = 864
  Height = 700
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Echo List'
  Color = clBtnFace
  Constraints.MinHeight = 339
  Constraints.MinWidth = 583
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnResize = FormResize
  DesignSize = (
    856
    666)
  PixelsPerInch = 96
  TextHeight = 13
  object SelectedLabel: TLabel
    Left = 16
    Top = 9
    Width = 337
    Height = 33
    Alignment = taCenter
    AutoSize = False
    Caption = 'SelectedLabel'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    Layout = tlCenter
  end
  object Bevel1: TBevel
    Left = 8
    Top = 9
    Width = 353
    Height = 33
  end
  object Label8: TLabel
    Left = 8
    Top = 632
    Width = 385
    Height = 13
    Anchors = [akLeft, akBottom]
    AutoSize = False
    Caption = 
      'Double click echo to view; double click '#39'Comment'#39' field to edit ' +
      'report.'
  end
  object CloseButton: TButton
    Left = 769
    Top = 639
    Width = 81
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Close'
    TabOrder = 1
    OnClick = CloseButtonClick
  end
  object DisplayEchosGroupBox: TGroupBox
    Left = 376
    Top = 4
    Width = 193
    Height = 181
    Caption = 'Display Echos'
    TabOrder = 0
    object ReadCountLabel: TLabel
      Left = 120
      Top = 22
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object NotReadCountLabel: TLabel
      Left = 120
      Top = 48
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object OnlineCountLabel: TLabel
      Left = 120
      Top = 74
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object OfflineCountLabel: TLabel
      Left = 120
      Top = 100
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object BackedUpCountLabel: TLabel
      Left = 120
      Top = 126
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object NotBackedUpCountLabel: TLabel
      Left = 120
      Top = 152
      Width = 57
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = '0'
      Layout = tlCenter
    end
    object NotReadCheckBox: TCheckBox
      Left = 16
      Top = 48
      Width = 97
      Height = 17
      Caption = 'Not Read'
      TabOrder = 1
      OnClick = NotReadCheckBoxClick
    end
    object ReadCheckBox: TCheckBox
      Left = 16
      Top = 22
      Width = 97
      Height = 17
      Caption = 'Read'
      TabOrder = 0
      OnClick = ReadCheckBoxClick
    end
    object OnlineCheckBox: TCheckBox
      Left = 16
      Top = 74
      Width = 73
      Height = 17
      Caption = 'Online'
      TabOrder = 2
      OnClick = OnlineCheckBoxClick
    end
    object OfflineCheckBox: TCheckBox
      Left = 16
      Top = 100
      Width = 73
      Height = 17
      Caption = 'Offline'
      TabOrder = 3
      OnClick = OfflineCheckBoxClick
    end
    object BackedUpCheckBox: TCheckBox
      Left = 16
      Top = 126
      Width = 97
      Height = 17
      Caption = 'Backed Up'
      TabOrder = 4
      OnClick = BackedUpCheckBoxClick
    end
    object NotBackedUpCheckBox: TCheckBox
      Left = 16
      Top = 152
      Width = 97
      Height = 17
      Caption = 'Not Backed Up'
      TabOrder = 5
      OnClick = NotBackedUpCheckBoxClick
    end
  end
  object EchoListPanel: TPanel
    Left = 8
    Top = 192
    Width = 840
    Height = 425
    Anchors = [akLeft, akTop, akRight, akBottom]
    Caption = 'EchoListPanel'
    TabOrder = 2
    DesignSize = (
      840
      425)
    object EchoListView: TListView
      Left = 0
      Top = 0
      Width = 840
      Height = 426
      Anchors = [akLeft, akTop, akRight, akBottom]
      Columns = <
        item
          Caption = 'Read Status'
          Width = 100
        end
        item
          Caption = 'Study Name'
          Width = 150
        end
        item
          Caption = 'Patient'
          Width = 150
        end
        item
          Alignment = taRightJustify
          Caption = 'Date'
          Width = 100
        end
        item
          Alignment = taCenter
          Caption = 'Read By'
          Width = 0
        end
        item
          Alignment = taCenter
          Caption = 'Online'
          Width = 45
        end
        item
          Alignment = taCenter
          Caption = 'Backup'
          Width = 52
        end
        item
          Alignment = taRightJustify
          Caption = 'ID'
        end
        item
          Alignment = taCenter
          Caption = 'Sonographer'
          Width = 0
        end
        item
          Caption = 'Comment'
          Width = 250
        end>
      LargeImages = MainForm.ImageListColorSquares
      ReadOnly = True
      RowSelect = True
      PopupMenu = EchoListPopup
      SmallImages = MainForm.ImageListColorSquares
      TabOrder = 0
      ViewStyle = vsReport
      OnChange = EchoListViewChange
      OnColumnClick = EchoListViewColumnClick
      OnDblClick = EchoListViewDblClick
      OnMouseDown = EchoListViewMouseDown
    end
  end
  object PageControl: TPageControl
    Left = 8
    Top = 48
    Width = 353
    Height = 137
    ActivePage = TabSheetAll
    TabHeight = 17
    TabIndex = 0
    TabOrder = 3
    OnChange = PageControlChange
    object TabSheetAll: TTabSheet
      Caption = 'All'
      object PatientSelectRadioGroup: TRadioGroup
        Left = 4
        Top = 0
        Width = 337
        Height = 53
        Caption = 'Patients'
        Items.Strings = (
          'All'
          'Selected')
        TabOrder = 0
        OnClick = PatientSelectRadioGroupClick
      end
      object PatientEdit: TEdit
        Left = 80
        Top = 26
        Width = 252
        Height = 21
        ReadOnly = True
        TabOrder = 1
        OnKeyDown = PatientEditKeyDown
      end
      object PatientListButton: TBitBtn
        Left = 313
        Top = 28
        Width = 17
        Height = 17
        TabOrder = 2
        OnClick = PatientListButtonClick
        Glyph.Data = {
          5A000000424D5A000000000000003E0000002800000007000000070000000100
          0100000000001C0000000000000000000000020000000200000000000000FFFF
          FF00FE000000FE000000EE000000C60000008200000000000000FE000000}
      end
      object ReadEchosBackGroupBox: TGroupBox
        Left = 4
        Top = 55
        Width = 153
        Height = 49
        Caption = 'Get Read Echos Back'
        TabOrder = 3
        object ReadBackComboBox: TComboBox
          Left = 8
          Top = 16
          Width = 137
          Height = 21
          AutoDropDown = True
          Style = csDropDownList
          ItemHeight = 13
          ItemIndex = 1
          MaxLength = 20
          TabOrder = 0
          Text = '3 Months'
          OnChange = ReadBackComboBoxChange
          Items.Strings = (
            '1 Month'
            '3 Months'
            '6 Months'
            '1 Year'
            '2 Years'
            '5 Years'
            'All')
        end
      end
      object SearchForGroupBox: TGroupBox
        Left = 164
        Top = 55
        Width = 177
        Height = 49
        Caption = 'Search For'
        TabOrder = 4
        object SearchEdit: TEdit
          Left = 8
          Top = 16
          Width = 113
          Height = 21
          AutoSelect = False
          MaxLength = 32
          TabOrder = 0
          Text = 'SearchEdit'
          OnChange = SearchEditChange
        end
        object SearchClearButton: TButton
          Left = 128
          Top = 16
          Width = 40
          Height = 21
          Caption = 'Clear'
          TabOrder = 1
          OnClick = SearchClearButtonClick
        end
      end
    end
    object TabSheetCD: TTabSheet
      Caption = 'CD'
      ImageIndex = 1
      object Label7: TLabel
        Left = 8
        Top = 8
        Width = 57
        Height = 21
        AutoSize = False
        Caption = 'Echo CD #'
        Layout = tlCenter
      end
      object CDEdit: TEdit
        Left = 72
        Top = 8
        Width = 65
        Height = 21
        AutoSize = False
        TabOrder = 0
        OnKeyDown = CDEditKeyDown
      end
      object DisplayCDButton: TButton
        Left = 152
        Top = 8
        Width = 113
        Height = 22
        Caption = 'Update Echo  List'
        TabOrder = 1
        OnClick = DisplayCDButtonClick
      end
      object SuggestButton: TButton
        Left = 8
        Top = 40
        Width = 209
        Height = 25
        Caption = 'Suggest CD with Echos to Make Offline'
        TabOrder = 2
        Visible = False
        OnClick = SuggestButtonClick
      end
      object OfflineButton: TButton
        Left = 8
        Top = 72
        Width = 209
        Height = 25
        Caption = 'Make All Displayed Echos Offline'
        TabOrder = 3
        Visible = False
        OnClick = OfflineButtonClick
      end
      object Button3: TButton
        Left = 224
        Top = 72
        Width = 113
        Height = 25
        Caption = 'Make Online'
        TabOrder = 4
        Visible = False
        OnClick = Button3Click
      end
    end
  end
  object PageControl1: TPageControl
    Left = 576
    Top = 8
    Width = 273
    Height = 177
    ActivePage = TabSheet2
    TabIndex = 0
    TabOrder = 4
    object TabSheet2: TTabSheet
      Caption = 'Echo Options'
      ImageIndex = 1
      object Button1: TButton
        Left = 16
        Top = 8
        Width = 153
        Height = 25
        Caption = 'New Echo Report'
        TabOrder = 0
        OnClick = Button1Click
      end
      object Button2: TButton
        Left = 16
        Top = 40
        Width = 153
        Height = 25
        Caption = 'Import Echos'
        TabOrder = 1
        OnClick = Button2Click
      end
      object Button_PatientAssign: TButton
        Left = 16
        Top = 72
        Width = 153
        Height = 25
        Caption = 'Assign Patients to Echos'
        TabOrder = 2
        OnClick = Button_PatientAssignClick
      end
    end
    object TabSheet1: TTabSheet
      Caption = 'CD Backup'
      object BackupGroupBox: TGroupBox
        Left = 8
        Top = 4
        Width = 249
        Height = 143
        Caption = 'Backup/Verify/Restore Progress'
        TabOrder = 0
        object BytesFreeLabel: TLabel
          Left = 88
          Top = 68
          Width = 65
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = '0'
          Layout = tlCenter
        end
        object CDIdLabel: TLabel
          Left = 80
          Top = 20
          Width = 73
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'CD ID Label'
          Layout = tlCenter
        end
        object ProgressGauge: TCGauge
          Left = 16
          Top = 118
          Width = 137
          Height = 17
          ForeColor = clBlue
        end
        object Label1: TLabel
          Left = 16
          Top = 20
          Width = 53
          Height = 13
          Caption = 'Echo CD #'
        end
        object Label2: TLabel
          Left = 16
          Top = 68
          Width = 50
          Height = 13
          Caption = 'Bytes Free'
        end
        object Label3: TLabel
          Left = 16
          Top = 84
          Width = 48
          Height = 13
          Caption = 'Echo Size'
        end
        object Label4: TLabel
          Left = 16
          Top = 100
          Width = 55
          Height = 13
          Caption = 'Bytes Done'
        end
        object EchoSizeLabel: TLabel
          Left = 88
          Top = 84
          Width = 65
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = '0'
          Layout = tlCenter
        end
        object BytesDoneLabel: TLabel
          Left = 88
          Top = 100
          Width = 65
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = '0'
          Layout = tlCenter
        end
        object Label5: TLabel
          Left = 16
          Top = 52
          Width = 30
          Height = 13
          Caption = 'Action'
        end
        object Label6: TLabel
          Left = 16
          Top = 36
          Width = 44
          Height = 13
          Caption = 'CD Label'
        end
        object ActionLabel: TLabel
          Left = 88
          Top = 52
          Width = 65
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'None'
          Layout = tlCenter
        end
        object CDLabelLabel: TLabel
          Left = 88
          Top = 36
          Width = 65
          Height = 13
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'N/A'
          Layout = tlCenter
        end
        object Bevel2: TBevel
          Left = 165
          Top = 16
          Width = 2
          Height = 118
        end
        object EjectButton: TButton
          Left = 176
          Top = 15
          Width = 65
          Height = 24
          Caption = 'Eject'
          TabOrder = 0
          OnClick = EjectButtonClick
        end
        object LoadButton: TButton
          Left = 176
          Top = 47
          Width = 65
          Height = 24
          Caption = 'Load'
          TabOrder = 1
          OnClick = LoadButtonClick
        end
        object CheckButton: TButton
          Left = 176
          Top = 79
          Width = 65
          Height = 24
          Caption = 'Check'
          TabOrder = 2
          OnClick = CheckButtonClick
        end
        object CancelButton: TButton
          Left = 176
          Top = 111
          Width = 65
          Height = 24
          Caption = 'Cancel'
          TabOrder = 3
          OnClick = CancelButtonClick
        end
      end
    end
  end
  object DatabaseCheckTimer: TTimer
    OnTimer = DatabaseCheckTimerTimer
    Left = 40
    Top = 8
  end
  object EchoListPopup: TPopupMenu
    OnPopup = EchoListPopupPopup
    Left = 8
    Top = 8
    object EchoListPopupRename: TMenuItem
      Caption = 'Rename '
      Visible = False
      OnClick = EchoListPopupRenameClick
    end
    object N7: TMenuItem
      Caption = '-'
    end
    object Popup_ViewEcho: TMenuItem
      Caption = 'View Echo'
      OnClick = Popup_ViewEchoClick
    end
    object N6: TMenuItem
      Caption = '-'
      Visible = False
    end
    object EchoListPopupDetails: TMenuItem
      Caption = 'Edit Report'
      OnClick = EchoListPopupDetailsClick
    end
    object Popup_PreviewReport: TMenuItem
      Caption = 'Preview Report'
      OnClick = Popup_PreviewReportClick
    end
    object N8: TMenuItem
      Caption = '-'
    end
    object Popup_Delete: TMenuItem
      Caption = 'Delete'
      Hint = 'Copy'
      ImageIndex = 0
      ShortCut = 16451
      OnClick = Popup_DeleteClick
    end
    object N4: TMenuItem
      Caption = '-'
    end
    object EchoListPopupPatientSet: TMenuItem
      Caption = 'Patient Set'
      OnClick = EchoListPopupPatientSetClick
    end
    object EchoListPopupPatientClear: TMenuItem
      Caption = 'Patient Clear'
      OnClick = EchoListPopupPatientClearClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object EchoListPopupMarkAsRead: TMenuItem
      Caption = 'Mark as Read'
      OnClick = EchoListPopupMarkAsReadClick
    end
    object EchoListPopupMarkAsNotRead: TMenuItem
      Caption = 'Mark as Not Read'
      OnClick = EchoListPopupMarkAsNotReadClick
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object EchoListPopupBackup: TMenuItem
      Caption = 'Backup'
      object EchoListPopupBackupDatabase: TMenuItem
        Caption = 'To Database CD'
        Visible = False
        OnClick = EchoListPopupBackupDatabaseClick
      end
      object EchoListPopupBackupNonDatabase: TMenuItem
        Caption = 'To Non-Database CD'
        OnClick = EchoListPopupBackupNonDatabaseClick
      end
    end
    object EchoListPopupBackupCDListGet: TMenuItem
      Caption = 'Get Backup CD List'
      OnClick = EchoListPopupBackupCDListGetClick
    end
    object EchoListPopupBackupVerify: TMenuItem
      Caption = 'Verify Backup'
      Visible = False
      OnClick = EchoListPopupBackupVerifyClick
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object EchoListPopupMakeOffline: TMenuItem
      Caption = 'Make Offline'
      Visible = False
      OnClick = EchoListPopupMakeOfflineClick
    end
    object EchoListPopupMakeOnline: TMenuItem
      Caption = 'Make Online (Restore from Backup)'
      OnClick = EchoListPopupMakeOnlineClick
    end
    object N5: TMenuItem
      Caption = '-'
    end
    object EchoListPopupComment: TMenuItem
      Caption = 'Edit Comment'
      Hint = 'Copy'
      ImageIndex = 0
      Visible = False
      OnClick = EditCommentClick
    end
  end
end
