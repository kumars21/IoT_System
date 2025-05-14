$(function() {
    const $form = $("#network-form-wifi");
    $("#network-btn-save").on("click", function() {
        $form.addClass('was-validated');
        if (!$form.get(0).checkValidity())
            return false;
    });
});