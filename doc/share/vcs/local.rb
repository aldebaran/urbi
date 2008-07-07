class Vcs

  # See http://rubyforge.org/projects/vcs
  # and http://vcs.rubyforge.org

  protocol_version '0.1'

  def local_commit! ( *args )
    common_commit!("share/ <%= rev %>: <%= title %>", *args) do |subject|
      mail!(:to => %w[lrde-patches@lrde.epita.fr], :subject => subject)
    end
  end
  default_commit :local_commit

end # class Vcs
