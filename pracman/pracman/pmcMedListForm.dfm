object MedListForm: TMedListForm
  Left = 434
  Top = 309
  Width = 457
  Height = 564
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Medications'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  DesignSize = (
    449
    537)
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 0
    Top = 0
    Width = 449
    Height = 537
    Anchors = [akLeft, akTop, akRight, akBottom]
    Visible = False
  end
  object ListView1: TListView
    Left = 8
    Top = 72
    Width = 433
    Height = 417
    Anchors = [akLeft, akTop, akRight, akBottom]
    Columns = <
      item
        Caption = 'Name'
      end>
    TabOrder = 0
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 145
    Height = 49
    Caption = 'Search'
    TabOrder = 1
    object SearchEdit: TEdit
      Left = 8
      Top = 16
      Width = 121
      Height = 21
      MaxLength = 127
      TabOrder = 0
    end
  end
  object GroupBox2: TGroupBox
    Left = 160
    Top = 8
    Width = 169
    Height = 49
    Caption = 'Selected'
    TabOrder = 2
    object SelectedLabel: TLabel
      Left = 8
      Top = 16
      Width = 153
      Height = 25
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
  end
  object GroupBox3: TGroupBox
    Left = 336
    Top = 8
    Width = 105
    Height = 49
    Caption = 'GroupBox3'
    TabOrder = 3
    object Button3: TButton
      Left = 8
      Top = 16
      Width = 89
      Height = 25
      Caption = 'New'
      TabOrder = 0
    end
  end
  object ButtonClose: TButton
    Left = 368
    Top = 504
    Width = 75
    Height = 25
    Caption = 'Close'
    TabOrder = 4
    OnClick = ButtonCloseClick
  end
end
