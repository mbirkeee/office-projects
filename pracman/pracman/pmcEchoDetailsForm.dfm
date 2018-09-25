object EchoDetailsForm: TEchoDetailsForm
  Left = 112
  Top = 119
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'EchoDetailsForm'
  ClientHeight = 586
  ClientWidth = 992
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object LabelChangeCount: TLabel
    Left = 624
    Top = 552
    Width = 97
    Height = 25
    AutoSize = False
    Caption = 'LabelChangeCount'
    Layout = tlCenter
    Visible = False
  end
  object Label71: TLabel
    Left = 8
    Top = 120
    Width = 57
    Height = 17
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Comment'
    Layout = tlCenter
  end
  object Label32: TLabel
    Left = 8
    Top = 152
    Width = 57
    Height = 17
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Indication'
    Layout = tlCenter
  end
  object Label72: TLabel
    Left = 648
    Top = 152
    Width = 73
    Height = 17
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Image Quality'
    Layout = tlCenter
  end
  object Label108: TLabel
    Left = 8
    Top = 552
    Width = 177
    Height = 13
    AutoSize = False
    Caption = 'Press TAB to move to next field'
  end
  object PageControl1: TPageControl
    Left = 0
    Top = 192
    Width = 993
    Height = 345
    ActivePage = TabSheet_Measurements
    TabIndex = 0
    TabOrder = 7
    OnChange = PageControl1Change
    object TabSheet_Measurements: TTabSheet
      Caption = 'Measurements'
      object GroupBox_AorticValve: TGroupBox
        Left = 248
        Top = 8
        Width = 193
        Height = 297
        Caption = 'Aortic Valve'
        TabOrder = 1
        object Label39: TLabel
          Left = 24
          Top = 24
          Width = 41
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Regurg'
          Layout = tlCenter
        end
        object Label40: TLabel
          Left = 16
          Top = 48
          Width = 49
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Stenosis'
          Layout = tlCenter
        end
        object Label41: TLabel
          Left = 24
          Top = 73
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'AR jet/LVOT'
          Layout = tlCenter
        end
        object Label42: TLabel
          Left = 8
          Top = 96
          Width = 89
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'AR pres. half time'
          Layout = tlCenter
        end
        object Label43: TLabel
          Left = 24
          Top = 120
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Max. velocity'
          Layout = tlCenter
        end
        object Label44: TLabel
          Left = 24
          Top = 144
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Peak gradient'
          Layout = tlCenter
        end
        object Label45: TLabel
          Left = 24
          Top = 168
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Mean gradient'
          Layout = tlCenter
        end
        object Label46: TLabel
          Left = 24
          Top = 192
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LVOT diameter'
          Layout = tlCenter
        end
        object Label47: TLabel
          Left = 24
          Top = 216
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LVOT velocity'
          Layout = tlCenter
        end
        object Label48: TLabel
          Left = 24
          Top = 240
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'VTI'
          Layout = tlCenter
        end
        object Label49: TLabel
          Left = 24
          Top = 264
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Valve area'
          Layout = tlCenter
        end
        object Label50: TLabel
          Left = 152
          Top = 72
          Width = 33
          Height = 17
          AutoSize = False
          Caption = '%'
          Layout = tlCenter
        end
        object Label51: TLabel
          Left = 152
          Top = 96
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'ms'
          Layout = tlCenter
        end
        object Label52: TLabel
          Left = 152
          Top = 120
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label53: TLabel
          Left = 152
          Top = 144
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'mmHg'
          Layout = tlCenter
        end
        object Label54: TLabel
          Left = 152
          Top = 168
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'mmHg'
          Layout = tlCenter
        end
        object Label55: TLabel
          Left = 152
          Top = 192
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label56: TLabel
          Left = 152
          Top = 216
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label57: TLabel
          Left = 152
          Top = 240
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 's'
          Layout = tlCenter
        end
        object Label58: TLabel
          Left = 152
          Top = 264
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label59: TLabel
          Left = 168
          Top = 261
          Width = 6
          Height = 13
          Caption = '2'
        end
        object Edit_av_apht: TEdit
          Left = 104
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
        object Edit_av_ajl: TEdit
          Left = 104
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_av_mv: TEdit
          Left = 104
          Top = 120
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 4
          OnChange = EchoDetailsChange
        end
        object Edit_av_pg: TEdit
          Left = 104
          Top = 144
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 5
          OnChange = EchoDetailsChange
        end
        object Edit_av_mg: TEdit
          Left = 104
          Top = 168
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 6
          OnChange = EchoDetailsChange
        end
        object Edit_av_ld: TEdit
          Left = 104
          Top = 192
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 7
          OnChange = EchoDetailsChange
        end
        object Edit_av_lv: TEdit
          Left = 104
          Top = 216
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 8
          OnChange = EchoDetailsChange
        end
        object Edit_av_vti: TEdit
          Left = 104
          Top = 240
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 9
          OnChange = EchoDetailsChange
        end
        object Edit_av_va: TEdit
          Left = 104
          Top = 264
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 10
          OnChange = EchoDetailsChange
        end
        object ComboBox_av_reg: TComboBox
          Left = 72
          Top = 24
          Width = 113
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          Text = 'ComboBox_av_reg'
          OnChange = EchoDetailsChange
        end
        object ComboBox_av_sten: TComboBox
          Left = 72
          Top = 48
          Width = 113
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_MitralValve: TGroupBox
        Left = 448
        Top = 8
        Width = 177
        Height = 297
        Caption = 'Mitral Valve'
        TabOrder = 2
        object Label60: TLabel
          Left = 8
          Top = 24
          Width = 41
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Regurg'
          Layout = tlCenter
        end
        object Label61: TLabel
          Left = 8
          Top = 72
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'MR jet area'
          Layout = tlCenter
        end
        object Label62: TLabel
          Left = 2
          Top = 48
          Width = 47
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Stenosis'
          Layout = tlCenter
        end
        object Label63: TLabel
          Left = 8
          Top = 96
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Valve area'
          Layout = tlCenter
        end
        object Label64: TLabel
          Left = 8
          Top = 120
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Peak E velocity'
          Layout = tlCenter
        end
        object Label65: TLabel
          Left = 8
          Top = 144
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Peak A velocity'
          Layout = tlCenter
        end
        object Label66: TLabel
          Left = 8
          Top = 168
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Mean gradient'
          Layout = tlCenter
        end
        object Label67: TLabel
          Left = 8
          Top = 192
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'IVRT'
          Layout = tlCenter
        end
        object Label68: TLabel
          Left = 8
          Top = 216
          Width = 73
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'E decel. time'
          Layout = tlCenter
        end
        object Label79: TLabel
          Left = 136
          Top = 72
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label80: TLabel
          Left = 136
          Top = 96
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label81: TLabel
          Left = 136
          Top = 216
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'ms'
          Layout = tlCenter
        end
        object Label82: TLabel
          Left = 136
          Top = 192
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 's'
          Layout = tlCenter
        end
        object Label83: TLabel
          Left = 136
          Top = 168
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'mmHg'
          Layout = tlCenter
        end
        object Label84: TLabel
          Left = 136
          Top = 144
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label85: TLabel
          Left = 136
          Top = 120
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label103: TLabel
          Left = 152
          Top = 93
          Width = 6
          Height = 13
          Caption = '2'
        end
        object Label107: TLabel
          Left = 152
          Top = 69
          Width = 6
          Height = 13
          Caption = '2'
        end
        object Edit_mv_mrja: TEdit
          Left = 88
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_mv_va: TEdit
          Left = 88
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
        object Edit_mv_pev: TEdit
          Left = 88
          Top = 120
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 4
          OnChange = EchoDetailsChange
        end
        object Edit_mv_pav: TEdit
          Left = 88
          Top = 144
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 5
          OnChange = EchoDetailsChange
        end
        object Edit_mv_mg: TEdit
          Left = 88
          Top = 168
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 6
          OnChange = EchoDetailsChange
        end
        object Edit_mv_ivrt: TEdit
          Left = 88
          Top = 192
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 7
          OnChange = EchoDetailsChange
        end
        object Edit_mv_edt: TEdit
          Left = 88
          Top = 216
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 8
          OnChange = EchoDetailsChange
        end
        object ComboBox_mv_reg: TComboBox
          Left = 56
          Top = 24
          Width = 113
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
        object ComboBox_mv_sten: TComboBox
          Left = 56
          Top = 48
          Width = 113
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_CardiacDimensions: TGroupBox
        Left = 0
        Top = 8
        Width = 241
        Height = 297
        Caption = 'Cardiac Dimensions'
        TabOrder = 0
        object Label1: TLabel
          Left = 8
          Top = 24
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Right Ventricle'
          Layout = tlCenter
        end
        object Label8: TLabel
          Left = 8
          Top = 48
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Ascending aorta'
          Layout = tlCenter
        end
        object Label9: TLabel
          Left = 8
          Top = 72
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Left atrium'
          Layout = tlCenter
        end
        object Label11: TLabel
          Left = 8
          Top = 96
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LV end-diastole'
          Layout = tlCenter
        end
        object Label12: TLabel
          Left = 8
          Top = 120
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LV end-systole'
          Layout = tlCenter
        end
        object Label13: TLabel
          Left = 8
          Top = 144
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Septum'
          Layout = tlCenter
        end
        object Label14: TLabel
          Left = 8
          Top = 168
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Post wall'
          Layout = tlCenter
        end
        object Label15: TLabel
          Left = 8
          Top = 192
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LV mass index'
          Layout = tlCenter
        end
        object Label16: TLabel
          Left = 8
          Top = 232
          Width = 81
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Est. LVEF'
          Layout = tlCenter
        end
        object Label17: TLabel
          Left = 144
          Top = 24
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label19: TLabel
          Left = 144
          Top = 48
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label20: TLabel
          Left = 144
          Top = 72
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label21: TLabel
          Left = 144
          Top = 96
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label22: TLabel
          Left = 144
          Top = 120
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label24: TLabel
          Left = 144
          Top = 144
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label25: TLabel
          Left = 144
          Top = 168
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label26: TLabel
          Left = 144
          Top = 192
          Width = 41
          Height = 17
          AutoSize = False
          Caption = 'gm/m'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
        end
        object Label27: TLabel
          Left = 144
          Top = 232
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '%'
          Layout = tlCenter
        end
        object Label28: TLabel
          Left = 173
          Top = 189
          Width = 6
          Height = 13
          Caption = '2'
        end
        object Label29: TLabel
          Left = 192
          Top = 24
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 3.0'
          Layout = tlCenter
        end
        object Label30: TLabel
          Left = 192
          Top = 48
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 3.7'
          Layout = tlCenter
        end
        object Label31: TLabel
          Left = 192
          Top = 72
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 4.0'
          Layout = tlCenter
        end
        object Label33: TLabel
          Left = 192
          Top = 96
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '3.5 - 5.7'
          Layout = tlCenter
        end
        object Label34: TLabel
          Left = 192
          Top = 120
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '2.5 - 4.0'
          Layout = tlCenter
        end
        object Label35: TLabel
          Left = 192
          Top = 144
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 1.1'
          Layout = tlCenter
        end
        object Label36: TLabel
          Left = 192
          Top = 168
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 1.1'
          Layout = tlCenter
        end
        object Label37: TLabel
          Left = 192
          Top = 192
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 131 M'
          Layout = tlCenter
        end
        object Label38: TLabel
          Left = 192
          Top = 208
          Width = 41
          Height = 17
          AutoSize = False
          Caption = '< 100 F'
          Layout = tlCenter
        end
        object Edit_cd_rv: TEdit
          Left = 96
          Top = 24
          Width = 41
          Height = 21
          AutoSize = False
          MaxLength = 4
          TabOrder = 0
          OnChange = EchoDetailsChange
        end
        object Edit_cd_aa: TEdit
          Left = 96
          Top = 48
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 1
          OnChange = EchoDetailsChange
        end
        object Edit_cd_lved: TEdit
          Left = 96
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
        object Edit_cd_la: TEdit
          Left = 96
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_cd_lves: TEdit
          Left = 96
          Top = 120
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 4
          OnChange = EchoDetailsChange
        end
        object Edit_cd_sept: TEdit
          Left = 96
          Top = 144
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 5
          OnChange = EchoDetailsChange
        end
        object Edit_cd_pw: TEdit
          Left = 96
          Top = 168
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 6
          OnChange = EchoDetailsChange
        end
        object Edit_cd_mi: TEdit
          Left = 96
          Top = 192
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 7
          OnChange = EchoDetailsChange
        end
        object Edit_cd_lvef: TEdit
          Left = 96
          Top = 232
          Width = 41
          Height = 21
          MaxLength = 5
          TabOrder = 8
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_PulmonicValve: TGroupBox
        Left = 632
        Top = 8
        Width = 169
        Height = 145
        Caption = 'Pulmonic Valve'
        TabOrder = 3
        object Label69: TLabel
          Left = 8
          Top = 24
          Width = 35
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Regurg'
          Layout = tlCenter
        end
        object Label73: TLabel
          Left = 8
          Top = 48
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Velocity'
          Layout = tlCenter
        end
        object Label74: TLabel
          Left = 8
          Top = 72
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'PAT'
          Layout = tlCenter
        end
        object Label75: TLabel
          Left = 8
          Top = 96
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Gradient'
          Layout = tlCenter
        end
        object Label76: TLabel
          Left = 128
          Top = 48
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label77: TLabel
          Left = 128
          Top = 72
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 's'
          Layout = tlCenter
        end
        object Label78: TLabel
          Left = 128
          Top = 96
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'mmHg'
          Layout = tlCenter
        end
        object ComboBox_pv_reg: TComboBox
          Left = 48
          Top = 24
          Width = 113
          Height = 21
          ItemHeight = 13
          MaxLength = 128
          TabOrder = 0
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
        object Edit_pv_vel: TEdit
          Left = 80
          Top = 48
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 1
          OnChange = EchoDetailsChange
        end
        object Edit_pv_pat: TEdit
          Left = 80
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_pv_grad: TEdit
          Left = 80
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_PulmonicVeinFlow: TGroupBox
        Left = 632
        Top = 160
        Width = 169
        Height = 145
        Caption = 'Pulmonic Vein Flow'
        TabOrder = 4
        object Label86: TLabel
          Left = 128
          Top = 96
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label87: TLabel
          Left = 128
          Top = 72
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label88: TLabel
          Left = 128
          Top = 48
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label89: TLabel
          Left = 128
          Top = 24
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label90: TLabel
          Left = 8
          Top = 24
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Systole'
          Layout = tlCenter
        end
        object Label91: TLabel
          Left = 8
          Top = 48
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Diastole'
          Layout = tlCenter
        end
        object Label92: TLabel
          Left = 8
          Top = 72
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'A reversal'
          Layout = tlCenter
        end
        object Label93: TLabel
          Left = 8
          Top = 96
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'LAA'
          Layout = tlCenter
        end
        object Edit_pvf_laa: TEdit
          Left = 80
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
        object Edit_pvf_ar: TEdit
          Left = 80
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_pvf_dia: TEdit
          Left = 80
          Top = 48
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 1
          OnChange = EchoDetailsChange
        end
        object Edit_pvf_sys: TEdit
          Left = 80
          Top = 24
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 0
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_TricuspidValve: TGroupBox
        Left = 808
        Top = 8
        Width = 177
        Height = 185
        Caption = 'Tricuspid Valve'
        TabOrder = 5
        object Label70: TLabel
          Left = 8
          Top = 24
          Width = 41
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Regurg'
          Layout = tlCenter
        end
        object Label94: TLabel
          Left = 16
          Top = 72
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'TR jet area'
          Layout = tlCenter
        end
        object Label95: TLabel
          Left = 16
          Top = 96
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'RVSP'
          Layout = tlCenter
        end
        object Label96: TLabel
          Left = 16
          Top = 120
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'E velocity'
          Layout = tlCenter
        end
        object Label97: TLabel
          Left = 2
          Top = 48
          Width = 47
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Stenosis'
          Layout = tlCenter
        end
        object Label98: TLabel
          Left = 16
          Top = 144
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Valve area'
          Layout = tlCenter
        end
        object Label99: TLabel
          Left = 136
          Top = 72
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label100: TLabel
          Left = 136
          Top = 144
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'cm'
          Layout = tlCenter
        end
        object Label101: TLabel
          Left = 136
          Top = 96
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'mmHg'
          Layout = tlCenter
        end
        object Label102: TLabel
          Left = 136
          Top = 120
          Width = 33
          Height = 17
          AutoSize = False
          Caption = 'm/s'
          Layout = tlCenter
        end
        object Label104: TLabel
          Left = 152
          Top = 69
          Width = 6
          Height = 13
          Caption = '2'
        end
        object Label105: TLabel
          Left = 152
          Top = 141
          Width = 6
          Height = 13
          Caption = '2'
        end
        object ComboBox_tv_reg: TComboBox
          Left = 56
          Top = 24
          Width = 113
          Height = 21
          ItemHeight = 13
          MaxLength = 128
          TabOrder = 0
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
        object Edit_tv_trja: TEdit
          Left = 88
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 2
          OnChange = EchoDetailsChange
        end
        object Edit_tv_rvsp: TEdit
          Left = 88
          Top = 96
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 3
          OnChange = EchoDetailsChange
        end
        object Edit_tv_ev: TEdit
          Left = 88
          Top = 120
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 4
          OnChange = EchoDetailsChange
        end
        object ComboBox_tv_sten: TComboBox
          Left = 56
          Top = 48
          Width = 113
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
        object Edit_tv_va: TEdit
          Left = 88
          Top = 144
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 5
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_RateRhythm: TGroupBox
        Left = 808
        Top = 248
        Width = 177
        Height = 57
        Caption = 'Rhythm'
        TabOrder = 7
        object ComboBox_rhythm: TComboBox
          Left = 8
          Top = 24
          Width = 161
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          Text = 'ComboBox1'
          OnChange = EchoDetailsChange
        end
      end
      object GroupBox_Rate: TGroupBox
        Left = 808
        Top = 200
        Width = 185
        Height = 41
        TabOrder = 6
        object Label106: TLabel
          Left = 16
          Top = 13
          Width = 65
          Height = 17
          Alignment = taRightJustify
          AutoSize = False
          Caption = 'Rate'
          Layout = tlCenter
        end
        object Edit_rate: TEdit
          Left = 88
          Top = 12
          Width = 41
          Height = 21
          MaxLength = 10
          TabOrder = 0
          OnChange = EchoDetailsChange
        end
      end
    end
    object TabSheet_Notes: TTabSheet
      Caption = 'Notes'
      ImageIndex = 1
      object Memo1: TMemo
        Left = 8
        Top = 8
        Width = 969
        Height = 297
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        Lines.Strings = (
          '')
        MaxLength = 100000
        ParentFont = False
        ScrollBars = ssVertical
        TabOrder = 0
        WantTabs = True
        OnChange = Memo1Change
      end
    end
  end
  object Button_Cancel: TButton
    Left = 912
    Top = 552
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 14
    OnClick = Button_CancelClick
  end
  object Edit_Comment: TEdit
    Left = 72
    Top = 120
    Width = 777
    Height = 21
    TabOrder = 4
    OnChange = EchoDetailsChange
  end
  object Button_Approve: TButton
    Left = 872
    Top = 80
    Width = 105
    Height = 25
    Caption = 'Approve and File'
    TabOrder = 10
    OnClick = Button_ApproveClick
  end
  object PatientGroupBox: TGroupBox
    Left = 8
    Top = 8
    Width = 201
    Height = 97
    Caption = 'Patient'
    TabOrder = 0
    object LabelPatientGender: TLabel
      Left = 168
      Top = 64
      Width = 17
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object Bevel24: TBevel
      Left = 160
      Top = 64
      Width = 33
      Height = 21
    end
    object PatientDobLabel: TLabel
      Left = 48
      Top = 64
      Width = 81
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object Bevel23: TBevel
      Left = 40
      Top = 64
      Width = 89
      Height = 21
    end
    object PatientPhnLabel: TLabel
      Left = 48
      Top = 40
      Width = 137
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object Bevel22: TBevel
      Left = 40
      Top = 40
      Width = 153
      Height = 21
    end
    object Label18: TLabel
      Left = 8
      Top = 42
      Width = 23
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'PHN'
      Layout = tlCenter
    end
    object Label23: TLabel
      Left = 8
      Top = 66
      Width = 23
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'DOB'
      Layout = tlCenter
    end
    object Edit_Patient: TEdit
      Left = 8
      Top = 16
      Width = 185
      Height = 21
      ReadOnly = True
      TabOrder = 0
      OnChange = EchoDetailsChange
    end
    object Button_PatientSelect: TBitBtn
      Left = 174
      Top = 18
      Width = 17
      Height = 17
      TabOrder = 1
      OnClick = Button_PatientSelectClick
      Glyph.Data = {
        5A000000424D5A000000000000003E0000002800000007000000070000000100
        0100000000001C0000000000000000000000020000000200000000000000FFFF
        FF00FE000000FE000000EE000000C60000008200000000000000FE000000}
    end
  end
  object ReferringDrGroupBox: TGroupBox
    Left = 384
    Top = 8
    Width = 209
    Height = 97
    Caption = 'Referring Doctor'
    TabOrder = 2
    object LabelDrFax: TLabel
      Left = 56
      Top = 64
      Width = 137
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object LabelDrPhone: TLabel
      Left = 56
      Top = 40
      Width = 137
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object Label7: TLabel
      Left = 8
      Top = 42
      Width = 33
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Phone'
      Layout = tlCenter
    end
    object Label10: TLabel
      Left = 8
      Top = 66
      Width = 33
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Fax'
      Layout = tlCenter
    end
    object Bevel5: TBevel
      Left = 48
      Top = 40
      Width = 153
      Height = 21
    end
    object Bevel6: TBevel
      Left = 48
      Top = 64
      Width = 153
      Height = 21
    end
    object Edit_ReferringDr: TEdit
      Left = 8
      Top = 16
      Width = 193
      Height = 21
      MaxLength = 64
      ReadOnly = True
      TabOrder = 0
      OnChange = EchoDetailsChange
    end
    object Button_ReferringDrSelect: TBitBtn
      Left = 182
      Top = 18
      Width = 17
      Height = 17
      TabOrder = 1
      OnClick = Button_ReferringDrSelectClick
      Glyph.Data = {
        5A000000424D5A000000000000003E0000002800000007000000070000000100
        0100000000001C0000000000000000000000020000000200000000000000FFFF
        FF00FE000000FE000000EE000000C60000008200000000000000FE000000}
    end
  end
  object ProviderGroupBox: TGroupBox
    Left = 216
    Top = 8
    Width = 161
    Height = 47
    Caption = 'Provider'
    TabOrder = 1
    object ProviderBillingNumberLabel: TLabel
      Left = 96
      Top = 64
      Width = 57
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object ComboBox_Provider: TComboBox
      Left = 8
      Top = 16
      Width = 145
      Height = 21
      ItemHeight = 13
      MaxLength = 128
      TabOrder = 0
      OnChange = ComboBox_ProviderChange
    end
  end
  object GroupBoxStudyDetails: TGroupBox
    Left = 600
    Top = 8
    Width = 249
    Height = 97
    Caption = 'Study Details'
    TabOrder = 3
    object LabelStudyDate: TLabel
      Left = 80
      Top = 40
      Width = 89
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object LabelReadDate: TLabel
      Left = 72
      Top = 64
      Width = 97
      Height = 21
      Alignment = taCenter
      AutoSize = False
      Layout = tlCenter
    end
    object Bevel3: TBevel
      Left = 72
      Top = 64
      Width = 97
      Height = 21
    end
    object Bevel2: TBevel
      Left = 72
      Top = 40
      Width = 97
      Height = 21
    end
    object LabelEchoId: TLabel
      Left = 192
      Top = 40
      Width = 49
      Height = 21
      Alignment = taCenter
      AutoSize = False
      Layout = tlCenter
    end
    object Label5: TLabel
      Left = 176
      Top = 40
      Width = 25
      Height = 21
      AutoSize = False
      Caption = 'ID'
      Layout = tlCenter
    end
    object Label3: TLabel
      Left = 24
      Top = 16
      Width = 41
      Height = 17
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Name'
      Layout = tlCenter
    end
    object Label4: TLabel
      Left = 8
      Top = 40
      Width = 57
      Height = 21
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Date'
      Layout = tlCenter
    end
    object Label6: TLabel
      Left = 8
      Top = 64
      Width = 57
      Height = 21
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Read Date'
      Layout = tlCenter
    end
    object Bevel1: TBevel
      Left = 192
      Top = 40
      Width = 49
      Height = 21
    end
    object Edit_StudyName: TEdit
      Left = 72
      Top = 16
      Width = 169
      Height = 21
      MaxLength = 250
      TabOrder = 0
      OnChange = EchoDetailsChange
    end
  end
  object Button_OK: TButton
    Left = 824
    Top = 552
    Width = 75
    Height = 25
    Caption = 'OK'
    TabOrder = 13
    OnClick = Button_OKClick
  end
  object Button_Preview: TButton
    Left = 872
    Top = 16
    Width = 105
    Height = 25
    Caption = 'View Report'
    TabOrder = 8
    OnClick = Button_PreviewClick
  end
  object Button_Submit: TButton
    Left = 872
    Top = 48
    Width = 105
    Height = 25
    Caption = 'Submit for Approval'
    TabOrder = 9
    OnClick = Button_SubmitClick
  end
  object Button_Print: TButton
    Left = 872
    Top = 112
    Width = 105
    Height = 25
    Caption = 'Print'
    Enabled = False
    TabOrder = 11
  end
  object Button_Save: TButton
    Left = 736
    Top = 552
    Width = 75
    Height = 25
    Caption = 'Save'
    TabOrder = 12
    OnClick = Button_SaveClick
  end
  object GroupBox_Sonographer: TGroupBox
    Left = 216
    Top = 57
    Width = 161
    Height = 48
    Caption = 'Sonographer'
    TabOrder = 15
    object Label2: TLabel
      Left = 96
      Top = 64
      Width = 57
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object LabelSonographer: TLabel
      Left = 16
      Top = 15
      Width = 137
      Height = 21
      AutoSize = False
      Layout = tlCenter
    end
    object Bevel4: TBevel
      Left = 8
      Top = 15
      Width = 145
      Height = 21
    end
  end
  object Edit_Indication: TEdit
    Left = 72
    Top = 152
    Width = 577
    Height = 21
    MaxLength = 1024
    TabOrder = 5
    OnChange = EchoDetailsChange
  end
  object ComboBox_ImageQuality: TComboBox
    Left = 728
    Top = 152
    Width = 121
    Height = 21
    ItemHeight = 13
    MaxLength = 128
    TabOrder = 6
    OnChange = EchoDetailsChange
  end
end
